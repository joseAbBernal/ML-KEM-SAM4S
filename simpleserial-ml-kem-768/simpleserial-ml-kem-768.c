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

#include "api.h"
#include "hal.h"
#include "simpleserial.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ML-KEM-768 key sizes
#define MLKEM768_PUBLIC_KEY_BYTES  1184
#define MLKEM768_SECRET_KEY_BYTES   2400
#define MLKEM768_CIPHERTEXT_BYTES  1088
#define MLKEM768_SHARED_SECRET_BYTES 32

static uint8_t sk[MLKEM768_SECRET_KEY_BYTES];
static uint8_t pk[MLKEM768_PUBLIC_KEY_BYTES];

/**
 * Generate keypair - command 'k'
 * Input: None
 * Output: Public key (pk) and stores secret key internally
 */
uint8_t get_key(uint8_t* k, uint8_t len)
{
    (void)len;
    (void)k;
    
    // Generate keypair
    crypto_kem_keypair(pk, sk);
    
    // Return public key
    simpleserial_put('r', MLKEM768_PUBLIC_KEY_BYTES, pk);
    return 0x00;
}

/**
 * Set secret key - command 's' 
 * Input: Secret key (sk)
 * Output: None
 */
uint8_t set_sk(uint8_t* sk_in, uint8_t len)
{
    if(len != MLKEM768_SECRET_KEY_BYTES)
        return 0x01;
    
    memcpy(sk, sk_in, MLKEM768_SECRET_KEY_BYTES);
    return 0x00;
}

/**
 * Set public key - command 'p'
 * Input: Public key (pk)
 * Output: None
 */
uint8_t set_pk(uint8_t* pk_in, uint8_t len)
{
    if(len != MLKEM768_PUBLIC_KEY_BYTES)
        return 0x01;
    
    memcpy(pk, pk_in, MLKEM768_PUBLIC_KEY_BYTES);
    return 0x00;
}

/**
 * Encrypt - command 'e'
 * Input: Public key (already set via 'p')
 * Output: Ciphertext (ct) and shared secret (ss)
 */
uint8_t do_enc(uint8_t* pt, uint8_t len)
{
    uint8_t ct[MLKEM768_CIPHERTEXT_BYTES];
    uint8_t ss[MLKEM768_SHARED_SECRET_BYTES];
    
    if(len != 0)
        return 0x01;
    
    trigger_high();
    
    // Encrypt using public key
    crypto_kem_enc(ct, ss, pk);
    
    trigger_low();
    
    // Return ciphertext and shared secret
    simpleserial_put('c', MLKEM768_CIPHERTEXT_BYTES, ct);
    simpleserial_put('s', MLKEM768_SHARED_SECRET_BYTES, ss);
    return 0x00;
}

/**
 * Decrypt - command 'd'
 * Input: Ciphertext (ct)
 * Output: Shared secret (ss)
 */
uint8_t do_dec(uint8_t* ct, uint8_t len)
{
    uint8_t ss[MLKEM768_SHARED_SECRET_BYTES];
    
    if(len != MLKEM768_CIPHERTEXT_BYTES)
        return 0x01;
    
    trigger_high();
    
    // Decrypt using secret key
    crypto_kem_dec(ss, ct, sk);
    
    trigger_low();
    
    // Return shared secret
    simpleserial_put('r', MLKEM768_SHARED_SECRET_BYTES, ss);
    return 0x00;
}

/**
 * Reset - command 'x'
 */
uint8_t reset(uint8_t* x, uint8_t len)
{
    (void)x;
    (void)len;
    // Reset keys if needed
    return 0x00;
}

#if SS_VER == SS_VER_2_1
uint8_t mlkem(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t req_len = 0;
    uint8_t err = 0;
    
    if (scmd & 0x01) {
        // Generate keypair
        crypto_kem_keypair(pk, sk);
        req_len = 0;
        simpleserial_put('r', MLKEM768_PUBLIC_KEY_BYTES, pk);
    }
    
    if (scmd & 0x02) {
        // Encrypt
        uint8_t ct[MLKEM768_CIPHERTEXT_BYTES];
        uint8_t ss[MLKEM768_SHARED_SECRET_BYTES];
        
        crypto_kem_enc(ct, ss, pk);
        
        // Build response: ct (1088 bytes) + ss (32 bytes)
        memcpy(buf + req_len, ct, MLKEM768_CIPHERTEXT_BYTES);
        req_len += MLKEM768_CIPHERTEXT_BYTES;
        memcpy(buf + req_len, ss, MLKEM768_SHARED_SECRET_BYTES);
        req_len += MLKEM768_SHARED_SECRET_BYTES;
    }

    return 0x00;
}
#endif

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();
    
    // Initialize keys to zero
    memset(pk, 0, MLKEM768_PUBLIC_KEY_BYTES);
    memset(sk, 0, MLKEM768_SECRET_KEY_BYTES);
    
    simpleserial_init();
    
#if SS_VER == SS_VER_2_1
    simpleserial_addcmd(0x01, 0, mlkem);  // Generate keypair
    simpleserial_addcmd(0x02, 0, mlkem);  // Encrypt
#else
    simpleserial_addcmd('k', 0, get_key);      // Generate keypair
    simpleserial_addcmd('p', MLKEM768_PUBLIC_KEY_BYTES, set_pk);   // Set public key
    simpleserial_addcmd('s', MLKEM768_SECRET_KEY_BYTES, set_sk);   // Set secret key
    simpleserial_addcmd('e', 0, do_enc);      // Encrypt
    simpleserial_addcmd('d', MLKEM768_CIPHERTEXT_BYTES, do_dec);  // Decrypt
    simpleserial_addcmd('x', 0, reset);       // Reset
#endif
    
    while(1)
        simpleserial_get();
}
