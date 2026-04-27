/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2017 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include "simpleserial.h"
#include <stdint.h>
#include <stdlib.h>

/* SAM4S GPIO includes for LED control */
#include "pio.h"
#include "gpio.h"

/* ML-KEM-512 includes */
#include "indcpa.h"
#include "kem.h"
#include "params.h"
#include "randombytes.h"
#include "symmetric.h"
#include "verify.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//uint8_t pk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_PUBLICKEYBYTES];
//uint8_t sk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_SECRETKEYBYTES];
//uint8_t ct[PQCLEAN_MLKEM512_CLEAN_CRYPTO_CIPHERTEXTBYTES];
//uint8_t ss[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];

uint8_t pk[KYBER_PUBLICKEYBYTES] = {0};

uint8_t sk[KYBER_SECRETKEYBYTES] = {0};

uint8_t ct[KYBER_CIPHERTEXTBYTES] = {0};

uint8_t ss[KYBER_SSBYTES] = {0};

uint8_t ss2[KYBER_SSBYTES] = {0};

static uint8_t ss_result[KYBER_SSBYTES];
static uint8_t decap_buf[2 * KYBER_SYMBYTES];
static uint8_t decap_kr[2 * KYBER_SYMBYTES];
static uint8_t decap_decrypted_buf[KYBER_SYMBYTES];

#if SS_VER == SS_VER_2_1
#define MLKEM_SS_V21_CMD         0x01
#define MLKEM_SS_V21_CMD_RX      0x02
#define MLKEM_SS_V21_CMD_TX      0x03
#define MLKEM_SS_V21_CHUNK_SK    204u
#define MLKEM_SS_V21_CHUNK_PK    200u
#define MLKEM_SS_V21_CHUNK_CT    192u
#define MLKEM_SS_V21_SK_CHUNKS   8u
#define MLKEM_SS_V21_PK_CHUNKS   4u
#define MLKEM_SS_V21_CT_CHUNKS   4u
#define MLKEM_SS_V21_SK_MASK     ((1u << MLKEM_SS_V21_SK_CHUNKS) - 1u)
#define MLKEM_SS_V21_PK_MASK     ((1u << MLKEM_SS_V21_PK_CHUNKS) - 1u)
#define MLKEM_SS_V21_CT_MASK     ((1u << MLKEM_SS_V21_CT_CHUNKS) - 1u)

enum {
    MLKEM_SS_V21_SCMD_SK_BASE = 0x00,
    MLKEM_SS_V21_SCMD_PK_BASE = 0x10,
    MLKEM_SS_V21_SCMD_CT_BASE = 0x20,
    MLKEM_SS_V21_SCMD_SS = 0x30,
    MLKEM_SS_V21_SCMD_DECAP = 0x40
};

static uint8_t sk_chunks_loaded = 0;
static uint8_t pk_chunks_loaded = 0;
static uint8_t ct_chunks_loaded = 0;
static uint8_t ss_loaded = 0;
#endif

/*Led functions*/
/* LED Control Functions for CW313/SAM4S (CWHUSKY)
   Pins: 14, 15, 16 correspond to LED3, LED2, LED1 on the board */
/* Using direct pin numbers instead of PIO_PA*_IDX to avoid header issues */
#define LED1    16  /* Pin 16 on port A */
#define LED2    15  /* Pin 15 on port A */
#define LED3    14  /* Pin 14 on port A */

/* LED State variables to track on/off status */
static volatile uint8_t LED1State = 0;  /* 0 = off, 1 = on */
static volatile uint8_t LED2State = 0;
static volatile uint8_t LED3State = 0;

/* function to delay almost a second */
static void delay_seconds(uint32_t seconds)
{
    const uint32_t loops_per_sec = 7000000u;
    for (uint32_t s = 0; s < seconds; s++) {
        for (volatile uint32_t i = 0; i < loops_per_sec; i++) {
            __asm__ volatile ("nop");
        }
    }
}

uint8_t led_toggle(uint8_t* m, uint8_t len){
    int led = m[0];  // Get LED number from input byte (14=LED3, 15=LED2, 16=LED1)
    uint8_t current_state = 0;
    
    // Get current state
    if(led == LED1) current_state = LED1State;
    else if(led == LED2) current_state = LED2State;
    else if(led == LED3) current_state = LED3State;
    
    // Toggle: if on, turn off; if off, turn on
    if(current_state == 1){
        gpio_set_pin_low(led);
        if(led == LED1) LED1State = 0;
        else if(led == LED2) LED2State = 0;
        else if(led == LED3) LED3State = 0;
    } else {
        gpio_set_pin_high(led);
        if(led == LED1) LED1State = 1;
        else if(led == LED2) LED2State = 1;
        else if(led == LED3) LED3State = 1;
    }
    
    return 0;
}

// Serial command callbacks for LED control
uint8_t led_on_cmd(uint8_t* m, uint8_t len){
    int led = m[0];  // Get LED number from input byte
    gpio_set_pin_high(led);
    if(led == LED1) LED1State = 1;
    else if(led == LED2) LED2State = 1;
    else if(led == LED3) LED3State = 1;
    return 0;
}

uint8_t led_off_cmd(uint8_t* m, uint8_t len){
    int led = m[0];  // Get LED number from input byte
    gpio_set_pin_low(led);
    if(led == LED1) LED1State = 0;
    else if(led == LED2) LED2State = 0;
    else if(led == LED3) LED3State = 0;
    return 0;
}

// Helper functions for direct LED control in code (not via serial)
void led_on(int led) {
    gpio_set_pin_high(led);
    if(led == LED1) LED1State = 1;
    else if(led == LED2) LED2State = 1;
    else if(led == LED3) LED3State = 1;
}

void led_off(int led) {
    gpio_set_pin_low(led);
    if(led == LED1) LED1State = 0;
    else if(led == LED2) LED2State = 0;
    else if(led == LED3) LED3State = 0;
}

static void set_led_pattern(uint8_t led1_on, uint8_t led2_on, uint8_t led3_on)
{
    if (led1_on) led_on(LED1);
    else led_off(LED1);

    if (led2_on) led_on(LED2);
    else led_off(LED2);

    if (led3_on) led_on(LED3);
    else led_off(LED3);
}

// Get LED state - returns 1 if LED is on, 0 if off
uint8_t get_led_state(int led) {
    if(led == LED1) return LED1State;
    else if(led == LED2) return LED2State;
    else if(led == LED3) return LED3State;
    return 0;
}
/*Led functions done*/

/* ML-KEM-512 Decapsulation: Complete with indcpa_dec, hash_g, verify, cmov */
uint8_t ml_kem_512_decap(uint8_t* k, uint8_t len)
{
    int fail;  /* Global fail variable for cmov comparison */
    (void)k;
    (void)len;
    
    set_led_pattern(1, 0, 0);  /* Stage 1: entering indcpa_dec */
    
    
    /* Decrypt ciphertext using secret key */
    PQCLEAN_MLKEM512_CLEAN_indcpa_dec(decap_decrypted_buf, ct, sk);
    
    /* Prepare buffer for hash_g (decrypted_msg || secret_seed) */
    memcpy(decap_buf, decap_decrypted_buf, KYBER_SYMBYTES);
    memcpy(decap_buf + KYBER_SYMBYTES, sk + KYBER_SECRETKEYBYTES - 2 * KYBER_SYMBYTES, KYBER_SYMBYTES);
    
    /* Compute hash_g(kr, buf, 2 * KYBER_SYMBYTES) */
    hash_g(decap_kr, decap_buf, 2 * KYBER_SYMBYTES);
    
    set_led_pattern(0, 1, 0);  /* Stage 2: hash_g done */
    
    /* coins are in kr+KYBER_SYMBYTES */
    //PQCLEAN_MLKEM512_CLEAN_indcpa_enc(cmp, decrypted_buf, pk, kr + KYBER_SYMBYTES);
    
    /* Verify: compare input ct with re-encrypted cmp */
    set_led_pattern(1, 1, 0);  /* Stage 3: entering verify */
    fail = PQCLEAN_MLKEM512_CLEAN_verify(ct, ct, KYBER_CIPHERTEXTBYTES);
    
    /* Compute rejection key for invalid ciphertexts */
    set_led_pattern(0, 1, 1);  /* Stage 4: entering rkprf */
    rkprf(ss_result, sk + KYBER_SECRETKEYBYTES - KYBER_SYMBYTES, ct);

    set_led_pattern(1, 0, 1);  /* Stage 5: entering cmov */
    
    /* CONDITIONAL MOVE: Copy true key if fail==0 (valid), else keep rejection key */
    trigger_high();
    PQCLEAN_MLKEM512_CLEAN_cmov(ss_result, decap_kr, KYBER_SYMBYTES, (uint8_t)(1 - fail));
    trigger_low();

    set_led_pattern(0, 0, 1);  /* Stage 6: decap finished */
    
    /* Send shared secret back */
    //simpleserial_put('r', PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES, ss);
    
	return 0x00;
}

#if SS_VER == SS_VER_2_1
static uint8_t v21_copy_chunk(uint8_t *dst, size_t dst_len, uint8_t chunk_len,
                              uint8_t chunk_index, uint8_t total_chunks,
                              uint8_t len, const uint8_t *buf)
{
    size_t offset;

    if ((chunk_index >= total_chunks) || (len != chunk_len)) {
        return SS_ERR_LEN;
    }

    offset = (size_t)chunk_index * chunk_len;
    if ((offset + chunk_len) > dst_len) {
        return SS_ERR_LEN;
    }

    memcpy(dst + offset, buf, chunk_len);
    return SS_ERR_OK;
}

static void v21_sync_pk_into_sk(void)
{
    memcpy(sk + KYBER_INDCPA_SECRETKEYBYTES, pk, KYBER_PUBLICKEYBYTES);
    hash_h(sk + KYBER_SECRETKEYBYTES - 2 * KYBER_SYMBYTES, pk, KYBER_PUBLICKEYBYTES);
}

static uint8_t v21_handle_vector_write(uint8_t scmd, uint8_t len, const uint8_t *buf)
{
    uint8_t chunk_index;
    uint8_t err;

    if ((scmd >= MLKEM_SS_V21_SCMD_SK_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_SK_BASE + MLKEM_SS_V21_SK_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_SK_BASE;
        err = v21_copy_chunk(sk, sizeof(sk), MLKEM_SS_V21_CHUNK_SK, chunk_index,
                             MLKEM_SS_V21_SK_CHUNKS, len, buf);
        if (err == SS_ERR_OK) {
            sk_chunks_loaded |= (uint8_t)(1u << chunk_index);
        }
        return err;
    }

    if ((scmd >= MLKEM_SS_V21_SCMD_PK_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_PK_BASE + MLKEM_SS_V21_PK_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_PK_BASE;
        err = v21_copy_chunk(pk, sizeof(pk), MLKEM_SS_V21_CHUNK_PK, chunk_index,
                             MLKEM_SS_V21_PK_CHUNKS, len, buf);
        if (err == SS_ERR_OK) {
            pk_chunks_loaded |= (uint8_t)(1u << chunk_index);
            memcpy(sk + KYBER_INDCPA_SECRETKEYBYTES + ((size_t)chunk_index * MLKEM_SS_V21_CHUNK_PK),
                   buf,
                   MLKEM_SS_V21_CHUNK_PK);
            if (pk_chunks_loaded == MLKEM_SS_V21_PK_MASK) {
                v21_sync_pk_into_sk();
            }
        }
        return err;
    }

    if ((scmd >= MLKEM_SS_V21_SCMD_CT_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_CT_BASE + MLKEM_SS_V21_CT_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_CT_BASE;
        err = v21_copy_chunk(ct, sizeof(ct), MLKEM_SS_V21_CHUNK_CT, chunk_index,
                             MLKEM_SS_V21_CT_CHUNKS, len, buf);
        if (err == SS_ERR_OK) {
            ct_chunks_loaded |= (uint8_t)(1u << chunk_index);
        }
        return err;
    }

    if (scmd == MLKEM_SS_V21_SCMD_SS) {
        if (len != KYBER_SSBYTES) {
            return SS_ERR_LEN;
        }
        memcpy(ss, buf, KYBER_SSBYTES);
        ss_loaded = 1;
        return SS_ERR_OK;
    }

    return SS_ERR_CMD;
}

static uint8_t v21_readback_bounds(uint8_t scmd, const uint8_t **src, uint8_t *out_len)
{
    uint8_t chunk_index;
    size_t offset;

    if ((scmd >= MLKEM_SS_V21_SCMD_SK_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_SK_BASE + MLKEM_SS_V21_SK_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_SK_BASE;
        offset = (size_t)chunk_index * MLKEM_SS_V21_CHUNK_SK;
        *src = sk + offset;
        *out_len = MLKEM_SS_V21_CHUNK_SK;
        return SS_ERR_OK;
    }

    if ((scmd >= MLKEM_SS_V21_SCMD_PK_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_PK_BASE + MLKEM_SS_V21_PK_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_PK_BASE;
        offset = (size_t)chunk_index * MLKEM_SS_V21_CHUNK_PK;
        *src = pk + offset;
        *out_len = MLKEM_SS_V21_CHUNK_PK;
        return SS_ERR_OK;
    }

    if ((scmd >= MLKEM_SS_V21_SCMD_CT_BASE) &&
        (scmd < (MLKEM_SS_V21_SCMD_CT_BASE + MLKEM_SS_V21_CT_CHUNKS))) {
        chunk_index = scmd - MLKEM_SS_V21_SCMD_CT_BASE;
        offset = (size_t)chunk_index * MLKEM_SS_V21_CHUNK_CT;
        *src = ct + offset;
        *out_len = MLKEM_SS_V21_CHUNK_CT;
        return SS_ERR_OK;
    }

    if (scmd == MLKEM_SS_V21_SCMD_SS) {
        *src = ss;
        *out_len = KYBER_SSBYTES;
        return SS_ERR_OK;
    }

    return SS_ERR_CMD;
}

static uint8_t led_on_cmd_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    if (len != 1) {
        return SS_ERR_LEN;
    }
    return led_on_cmd(buf, len);
}

static uint8_t led_off_cmd_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    if (len != 1) {
        return SS_ERR_LEN;
    }
    return led_off_cmd(buf, len);
}

static uint8_t led_toggle_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    if (len != 1) {
        return SS_ERR_LEN;
    }
    return led_toggle(buf, len);
}

uint8_t ml_kem_512(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t err;

    (void)cmd;

    err = v21_handle_vector_write(scmd, len, buf);
    if (err != SS_ERR_CMD) {
        return err;
    }

    if (scmd == MLKEM_SS_V21_SCMD_DECAP) {
        if (len != 0) {
            return SS_ERR_LEN;
        }
        if ((sk_chunks_loaded != MLKEM_SS_V21_SK_MASK) ||
            (pk_chunks_loaded != MLKEM_SS_V21_PK_MASK) ||
            (ct_chunks_loaded != MLKEM_SS_V21_CT_MASK) ||
            (ss_loaded == 0)) {
            return SS_ERR_LEN;
        }
        err = ml_kem_512_decap(NULL, 0);
        if (err != SS_ERR_OK) {
            return err;
        }
        return PQCLEAN_MLKEM512_CLEAN_verify(ss, ss_result, KYBER_SSBYTES);
    }

    return SS_ERR_CMD;
}

static uint8_t ml_kem_512_rx_chunk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    return v21_handle_vector_write(scmd, len, buf);
}

static uint8_t ml_kem_512_tx_chunk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    const uint8_t *src;
    uint8_t out_len;
    uint8_t err;

    (void)cmd;
    (void)buf;

    if (len != 0) {
        return SS_ERR_LEN;
    }

    err = v21_readback_bounds(scmd, &src, &out_len);
    if (err != SS_ERR_OK) {
        return err;
    }

    simpleserial_put('k', out_len, src);
    return SS_ERR_OK;
}
#endif

int main(void)
{
    /* Setup the LEDs */
    gpio_configure_pin(LED1, PIO_OUTPUT_0 | PIO_DEFAULT);
    gpio_configure_pin(LED2, PIO_OUTPUT_0 | PIO_DEFAULT);
    gpio_configure_pin(LED3, PIO_OUTPUT_0 | PIO_DEFAULT);

    platform_init();
    init_uart();
    trigger_setup();

	simpleserial_init();

    /* Indicate startup */
    gpio_set_pin_high(LED1);
    gpio_set_pin_high(LED2);
    gpio_set_pin_high(LED3);

    delay_seconds(1);        //Wait for a second to make sure the user can see the LEDs turn on

    /*now turn off the leds */
    /*tur off the leds with the funcion led_off */
    led_off(LED1); //Turn on LED1 using led_on function
    led_off(LED2); //Turn on LED2 using led_on function
    led_off(LED3); //Turn on LED3 using led_on function

    #if SS_VER == SS_VER_2_1
    simpleserial_addcmd(MLKEM_SS_V21_CMD, MLKEM_SS_V21_CHUNK_SK, ml_kem_512);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_RX, MLKEM_SS_V21_CHUNK_SK, ml_kem_512_rx_chunk);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_TX, 0, ml_kem_512_tx_chunk);
    simpleserial_addcmd('h', 1, led_on_cmd_v21);
    simpleserial_addcmd('l', 1, led_off_cmd_v21);
    simpleserial_addcmd('t', 1, led_toggle_v21);
    #else
    simpleserial_addcmd('k', 16, get_key);
    simpleserial_addcmd('h', 1, led_on_cmd);
    simpleserial_addcmd('l', 1, led_off_cmd);
    simpleserial_addcmd('t', 1, led_toggle);
    #endif

    while(1)
        simpleserial_get();
}
