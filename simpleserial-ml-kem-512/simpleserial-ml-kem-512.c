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
/* GPIO: SAM (ASF) vs otros (STM32, AVR, etc.) */
#if defined(__SAM4LS4C__) || defined(__SAM4LS4B__) || defined(__SAM4S__) || defined(__SAM4S2B__)
#include "pio.h"
#include "gpio.h"
#else
/* Stubs para plataformas sin ASF (STM32, AVR, etc.) */
#define gpio_set_pin_high(pin) do { (void)(pin); } while(0)
#define gpio_set_pin_low(pin)  do { (void)(pin); } while(0)
#endif

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

uint8_t pk[KYBER_PUBLICKEYBYTES] = {0};

uint8_t sk[KYBER_SECRETKEYBYTES] = {0};

uint8_t ct[KYBER_CIPHERTEXTBYTES] = {0};

uint8_t ss[KYBER_SSBYTES] = {0};

uint8_t ss2[KYBER_SSBYTES] = {0};

static uint8_t ss_result[KYBER_SSBYTES];
static uint8_t decap_buf[2 * KYBER_SYMBYTES];
static uint8_t decap_kr[2 * KYBER_SYMBYTES];
static uint8_t decap_decrypted_buf[KYBER_SYMBYTES];
static uint8_t cmov_probe_buf[KYBER_SECRETKEYBYTES];

#if SS_VER == SS_VER_2_1
#define MLKEM_SS_V21_CMD         0x01
#define MLKEM_SS_V21_CMD_RX      0x02
#define MLKEM_SS_V21_CMD_TX      0x03
#define MLKEM_SS_V21_CMD_SET_CHUNK 0x04
#define MLKEM_SS_V21_CMD_RX_PK   0x05
#define MLKEM_SS_V21_CMD_TX_PK   0x06
#define MLKEM_SS_V21_CMD_RX_CT   0x07
#define MLKEM_SS_V21_CMD_TX_CT   0x08
#define MLKEM_SS_V21_CMD_RX_SS   0x09
#define MLKEM_SS_V21_CMD_TX_SS   0x0A
#define MLKEM_SS_V21_CMD_DECAP   0x0B
#define MLKEM_SS_V21_CMD_CMOV_SK 0x0C
#define MLKEM_SS_V21_CHUNK_SK    204u
#define MLKEM_SS_V21_CHUNK_PK    200u
#define MLKEM_SS_V21_CHUNK_CT    192u
#define MLKEM_SS_V21_CHUNK_MAX   248u
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
static uint8_t v21_chunk_sz = MLKEM_SS_V21_CHUNK_SK;
#endif

/* LED GPIO utilities kept as generic board controls (not tied to ML-KEM flow). */
#define LED1    16
#define LED2    15
#define LED3    14

static volatile uint8_t LED1State = 0;
static volatile uint8_t LED2State = 0;
static volatile uint8_t LED3State = 0;

static uint8_t led_on_cmd(uint8_t* m, uint8_t len)
{
    int led;

    if (len != 1u) {
        return SS_ERR_LEN;
    }

    led = m[0];
    gpio_set_pin_high(led);
    if (led == LED1) LED1State = 1;
    else if (led == LED2) LED2State = 1;
    else if (led == LED3) LED3State = 1;
    return 0;
}

static uint8_t led_off_cmd(uint8_t* m, uint8_t len)
{
    int led;

    if (len != 1u) {
        return SS_ERR_LEN;
    }

    led = m[0];
    gpio_set_pin_low(led);
    if (led == LED1) LED1State = 0;
    else if (led == LED2) LED2State = 0;
    else if (led == LED3) LED3State = 0;
    return 0;
}

static uint8_t led_toggle(uint8_t* m, uint8_t len)
{
    int led;
    uint8_t state = 0;

    if (len != 1u) {
        return SS_ERR_LEN;
    }

    led = m[0];
    if (led == LED1) state = LED1State;
    else if (led == LED2) state = LED2State;
    else if (led == LED3) state = LED3State;

    if (state) {
        gpio_set_pin_low(led);
        if (led == LED1) LED1State = 0;
        else if (led == LED2) LED2State = 0;
        else if (led == LED3) LED3State = 0;
    } else {
        gpio_set_pin_high(led);
        if (led == LED1) LED1State = 1;
        else if (led == LED2) LED2State = 1;
        else if (led == LED3) LED3State = 1;
    }

    return 0;
}

/* ML-KEM-512 Decapsulation: Complete with indcpa_dec, hash_g, verify, cmov */
uint8_t ml_kem_512_decap(uint8_t* k, uint8_t len)
{
    int fail;  /* Global fail variable for cmov comparison */
    (void)k;
    (void)len;

    /* Decrypt ciphertext using secret key */
    PQCLEAN_MLKEM512_CLEAN_indcpa_dec(decap_decrypted_buf, ct, sk);
    
    /* Prepare buffer for hash_g (decrypted_msg || secret_seed) */
    memcpy(decap_buf, decap_decrypted_buf, KYBER_SYMBYTES);
    memcpy(decap_buf + KYBER_SYMBYTES, sk + KYBER_SECRETKEYBYTES - 2 * KYBER_SYMBYTES, KYBER_SYMBYTES);
    
    /* Compute hash_g(kr, buf, 2 * KYBER_SYMBYTES) */
    hash_g(decap_kr, decap_buf, 2 * KYBER_SYMBYTES);

    /* coins are in kr+KYBER_SYMBYTES */
    //PQCLEAN_MLKEM512_CLEAN_indcpa_enc(cmp, decrypted_buf, pk, kr + KYBER_SYMBYTES);

    /* Verify: compare input ct with re-encrypted cmp */
    fail = PQCLEAN_MLKEM512_CLEAN_verify(ct, ct, KYBER_CIPHERTEXTBYTES);

    /* Compute rejection key for invalid ciphertexts */
    rkprf(ss_result, sk + KYBER_SECRETKEYBYTES - KYBER_SYMBYTES, ct);

    /* CONDITIONAL MOVE: Copy true key if fail==0 (valid), else keep rejection key */
    trigger_high();
    PQCLEAN_MLKEM512_CLEAN_cmov(ss_result, decap_kr, KYBER_SYMBYTES, (uint8_t)(1 - fail));
    trigger_low();

    /* Send shared secret back */
    /* (gpio_set_pin_high(LED1) removido: trigger_high/low ya marcan la ventana) */
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

static uint8_t v21_rx_vector_chunk(uint8_t *dst, size_t dst_len, uint8_t scmd,
                                   uint8_t len, const uint8_t *buf)
{
    size_t offset;
    size_t remaining;

    if ((v21_chunk_sz == 0u) || (len == 0u) || (len > v21_chunk_sz)) {
        return SS_ERR_LEN;
    }

    offset = (size_t)scmd * (size_t)v21_chunk_sz;
    if (offset >= dst_len) {
        return SS_ERR_LEN;
    }

    remaining = dst_len - offset;
    if ((size_t)len > remaining) {
        return SS_ERR_LEN;
    }

    memcpy(dst + offset, buf, len);
    return SS_ERR_OK;
}

static uint8_t v21_tx_vector_chunk(uint8_t tag, const uint8_t *src, size_t src_len,
                                   uint8_t scmd, uint8_t len)
{
    size_t offset;
    size_t remaining;
    uint8_t out_len;

    if ((v21_chunk_sz == 0u) || (len != 0u)) {
        return SS_ERR_LEN;
    }

    offset = (size_t)scmd * (size_t)v21_chunk_sz;
    if (offset >= src_len) {
        return SS_ERR_LEN;
    }

    remaining = src_len - offset;
    out_len = (remaining > (size_t)v21_chunk_sz) ? v21_chunk_sz : (uint8_t)remaining;
    simpleserial_put(tag, out_len, src + offset);
    return SS_ERR_OK;
}

static uint8_t v21_set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint16_t new_size;

    (void)cmd;
    (void)scmd;
    if (len != 2u) {
        return SS_ERR_LEN;
    }

    new_size = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    if ((new_size < 1u) || (new_size > MLKEM_SS_V21_CHUNK_MAX)) {
        return SS_ERR_LEN;
    }

    v21_chunk_sz = (uint8_t)new_size;
    return SS_ERR_OK;
}

static uint8_t v21_rx_pk_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t err;
    (void)cmd;
    err = v21_rx_vector_chunk(pk, sizeof(pk), scmd, len, buf);
    if (err == SS_ERR_OK) {
        v21_sync_pk_into_sk();
    }
    return err;
}

static uint8_t v21_tx_pk_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)buf;
    return v21_tx_vector_chunk('p', pk, sizeof(pk), scmd, len);
}

static uint8_t v21_rx_ct_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    return v21_rx_vector_chunk(ct, sizeof(ct), scmd, len, buf);
}

static uint8_t v21_tx_ct_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)buf;
    return v21_tx_vector_chunk('c', ct, sizeof(ct), scmd, len);
}

static uint8_t v21_rx_ss_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t err;
    (void)cmd;
    err = v21_rx_vector_chunk(ss, sizeof(ss), scmd, len, buf);
    if (err == SS_ERR_OK) {
        ss_loaded = 1u;
    }
    return err;
}

static uint8_t v21_tx_ss_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)buf;
    return v21_tx_vector_chunk('s', ss, sizeof(ss), scmd, len);
}

static uint8_t led_on_cmd_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    return led_on_cmd(buf, len);
}

static uint8_t led_off_cmd_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    return led_off_cmd(buf, len);
}

static uint8_t led_toggle_v21(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)scmd;
    return led_toggle(buf, len);
}

static uint8_t ml_kem_512_do_decap(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t err;
    (void)cmd; (void)scmd; (void)buf;
    if (len != 0) return SS_ERR_LEN;
    /* No bitmask check: the rx commands (0x05/0x07/0x09) use v21_rx_vector_chunk
     * which does not update chunk_loaded bitmasks. If vectors are unloaded the
     * verify at the end will return 0x01 (mismatch), which is the correct result. */
    err = ml_kem_512_decap(NULL, 0);
    if (err != SS_ERR_OK) return err;
    return PQCLEAN_MLKEM512_CLEAN_verify(ss, ss_result, KYBER_SSBYTES);
}

static uint8_t ml_kem_512_cmov_sk_probe(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t repeat;

    (void)cmd;
    (void)scmd;
    (void)buf;

    if (len != 0u) {
        return SS_ERR_LEN;
    }

    /*
     * Dedicated leakage probe: copy the complete secret key with cmov under a
     * single trigger so the capture covers the whole operation, not only a
     * narrow window around the no-fail decapsulation path.
     */
    trigger_high();
    for (repeat = 0; repeat < 4u; repeat++) {
        memset(cmov_probe_buf, 0, sizeof(cmov_probe_buf));
        PQCLEAN_MLKEM512_CLEAN_cmov(cmov_probe_buf, sk, sizeof(sk), 1u);
    }
    trigger_low();

    return SS_ERR_OK;
}

uint8_t ml_kem_512(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t err;

    (void)cmd;

    err = v21_handle_vector_write(scmd, len, buf);
    if (err != SS_ERR_CMD) {
        return err;
    }

    return SS_ERR_CMD;
}

static uint8_t ml_kem_512_rx_chunk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    return v21_rx_vector_chunk(sk, sizeof(sk), scmd, len, buf);
}

static uint8_t ml_kem_512_tx_chunk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    (void)cmd;
    (void)buf;
    return v21_tx_vector_chunk('k', sk, sizeof(sk), scmd, len);
}
#endif

int main(void)
{
    platform_init();
#if defined(__SAM4LS4C__) || defined(__SAM4LS4B__) || defined(__SAM4S__) || defined(__SAM4S2B__)
    gpio_configure_pin(LED1, PIO_OUTPUT_0 | PIO_DEFAULT);
    gpio_configure_pin(LED2, PIO_OUTPUT_0 | PIO_DEFAULT);
    gpio_configure_pin(LED3, PIO_OUTPUT_0 | PIO_DEFAULT);
#endif
    init_uart();
    trigger_setup();

	simpleserial_init();

    #if SS_VER == SS_VER_2_1
    simpleserial_addcmd(MLKEM_SS_V21_CMD, MLKEM_SS_V21_CHUNK_SK, ml_kem_512);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_RX, MLKEM_SS_V21_CHUNK_MAX, ml_kem_512_rx_chunk);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_TX, 0, ml_kem_512_tx_chunk);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_SET_CHUNK, 2, v21_set_chunk_size);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_RX_PK, MLKEM_SS_V21_CHUNK_MAX, v21_rx_pk_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_TX_PK, 0, v21_tx_pk_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_RX_CT, MLKEM_SS_V21_CHUNK_MAX, v21_rx_ct_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_TX_CT, 0, v21_tx_ct_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_RX_SS, MLKEM_SS_V21_CHUNK_MAX, v21_rx_ss_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_TX_SS, 0, v21_tx_ss_cmd);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_DECAP, 0, ml_kem_512_do_decap);
    simpleserial_addcmd(MLKEM_SS_V21_CMD_CMOV_SK, 0, ml_kem_512_cmov_sk_probe);
    simpleserial_addcmd('h', 1, led_on_cmd_v21);
    simpleserial_addcmd('l', 1, led_off_cmd_v21);
    simpleserial_addcmd('t', 1, led_toggle_v21);
    #else
    simpleserial_addcmd('k', 16, get_key);
    simpleserial_addcmd('h', 1, led_on_cmd);
    simpleserial_addcmd('l', 1, led_off_cmd);
    simpleserial_addcmd('t', 1, led_toggle);
    #endif

    gpio_set_pin_low(LED2);
    gpio_set_pin_low(LED1);
    gpio_set_pin_low(LED3);

    while(1)
        simpleserial_get();
}
