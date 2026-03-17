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
extern "C" {
    #include "api.h"
    #include "hal.h"
    #include "simpleserial.h"
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>
}

// ML-KEM-512 key sizes
#define KEM_PUBLIC_KEY_BYTES  KYBER_PUBLICKEYBYTES   // 800 bytes
#define KEM_SECRET_KEY_BYTES  KYBER_SECRETKEYBYTES   // 1632 bytes  
#define KEM_CIPHERTEXT_BYTES  KYBER_CIPHERTEXTBYTES  // 768 bytes
#define KEM_SHARED_SECRET_BYTES KYBER_SSBYTES         // 32 bytes

// Storage for KEM keys (static to avoid stack issues)
static uint8_t kem_public_key[KEM_PUBLIC_KEY_BYTES];
static uint8_t kem_secret_key[KEM_SECRET_KEY_BYTES];
static uint8_t kem_ciphertext[KEM_CIPHERTEXT_BYTES];
static uint8_t kem_shared_secret[KEM_SHARED_SECRET_BYTES];

// Key generation command
uint8_t kem_keygen(uint8_t* x, uint8_t len)
{
    (void)len;
    (void)x;
    
    // Generate key pair
    crypto_kem_keypair(kem_public_key, kem_secret_key);
    
    // Return public key
    simpleserial_put('p', KEM_PUBLIC_KEY_BYTES, kem_public_key);
    return 0x00;
}

// Encapsulation command (using public key)
uint8_t kem_encaps(uint8_t* pt, uint8_t len)
{
    // Input should be public key (or use stored public key if empty)
    if (len == KEM_PUBLIC_KEY_BYTES) {
        memcpy(kem_public_key, pt, KEM_PUBLIC_KEY_BYTES);
    } else if (len != 0) {
        return SS_ERR_LEN;
    }
    
    trigger_high();
    
    // Encapsulate: generates ciphertext and shared secret
    crypto_kem_enc(kem_ciphertext, kem_shared_secret, kem_public_key);
    
    trigger_low();
    
    // Return ciphertext + shared secret (for capture)
    simpleserial_put('c', KEM_CIPHERTEXT_BYTES + KEM_SHARED_SECRET_BYTES, kem_ciphertext);
    return 0x00;
}

// Decapsulation command (using secret key and ciphertext)
uint8_t kem_decaps(uint8_t* ct, uint8_t len)
{
    // Input should be ciphertext
    if (len != KEM_CIPHERTEXT_BYTES) {
        return SS_ERR_LEN;
    }
    
    memcpy(kem_ciphertext, ct, KEM_CIPHERTEXT_BYTES);
    
    trigger_high();
    
    // Decapsulate: uses secret key and ciphertext to recover shared secret
    crypto_kem_dec(kem_shared_secret, kem_ciphertext, kem_secret_key);
    
    trigger_low();
    
    // Return shared secret
    simpleserial_put('s', KEM_SHARED_SECRET_BYTES, kem_shared_secret);
    return 0x00;
}

// Set public key command
uint8_t get_pk(uint8_t* pk, uint8_t len)
{
    if (len != KEM_PUBLIC_KEY_BYTES) {
        return SS_ERR_LEN;
    }
    memcpy(kem_public_key, pk, KEM_PUBLIC_KEY_BYTES);
    return 0x00;
}

// Set secret key command
uint8_t get_sk(uint8_t* sk, uint8_t len)
{
    if (len != KEM_SECRET_KEY_BYTES) {
        return SS_ERR_LEN;
    }
    memcpy(kem_secret_key, sk, KEM_SECRET_KEY_BYTES);
    return 0x00;
}

// Reset command
uint8_t reset(uint8_t* x, uint8_t len)
{
    (void)x;
    (void)len;
    // Clear keys on reset
    memset(kem_public_key, 0, KEM_PUBLIC_KEY_BYTES);
    memset(kem_secret_key, 0, KEM_SECRET_KEY_BYTES);
    memset(kem_ciphertext, 0, KEM_CIPHERTEXT_BYTES);
    memset(kem_shared_secret, 0, KEM_SHARED_SECRET_BYTES);
    return 0x00;
}

// SS_VER_2_1 unified command handler
// Supports combined operations via subcommand flags:
// 0x01: Generate new keypair
// 0x02: Set public key (pk must be in buffer)
// 0x04: Set secret key (sk must be in buffer)
// 0x08: Encapsulate (requires pk in buffer or uses stored pk)
// 0x10: Decapsulate (requires sk and ct in buffer)
#if SS_VER == SS_VER_2_1
uint8_t kem(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t req_len = 0;
    uint8_t err = 0;
    
    (void)cmd;
    (void)err;
    
    // Handle key generation (0x01)
    if (scmd & 0x01) {
        trigger_high();
        crypto_kem_keypair(kem_public_key, kem_secret_key);
        trigger_low();
        // Return public key
        simpleserial_put('p', KEM_PUBLIC_KEY_BYTES, kem_public_key);
        return 0x00;
    }
    
    // Handle set public key (0x02)
    if (scmd & 0x02) {
        req_len += KEM_PUBLIC_KEY_BYTES;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        memcpy(kem_public_key, buf + req_len - KEM_PUBLIC_KEY_BYTES, KEM_PUBLIC_KEY_BYTES);
    }
    
    // Handle set secret key (0x04)
    if (scmd & 0x04) {
        req_len += KEM_SECRET_KEY_BYTES;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        memcpy(kem_secret_key, buf + req_len - KEM_SECRET_KEY_BYTES, KEM_SECRET_KEY_BYTES);
    }
    
    // Handle encapsulate (0x08)
    if (scmd & 0x08) {
        req_len += KEM_PUBLIC_KEY_BYTES;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        // If pk is provided in buffer, use it; otherwise use stored pk
        const uint8_t *pk_to_use = (scmd & 0x02) ? (buf) : (kem_public_key);
        
        trigger_high();
        crypto_kem_enc(kem_ciphertext, kem_shared_secret, pk_to_use);
        trigger_low();
        
        // Return ct + ss
        simpleserial_put('c', KEM_CIPHERTEXT_BYTES + KEM_SHARED_SECRET_BYTES, kem_ciphertext);
        return 0x00;
    }
    
    // Handle decapsulate (0x10)
    if (scmd & 0x10) {
        req_len += KEM_CIPHERTEXT_BYTES;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        // If ct is provided in buffer, use it; otherwise use stored ct
        uint8_t *ct_to_use = (scmd & 0x08) ? (buf + ((scmd & 0x02) ? KEM_PUBLIC_KEY_BYTES : 0)) : (kem_ciphertext);
        if (!(scmd & 0x08)) {
            memcpy(kem_ciphertext, buf + req_len - KEM_CIPHERTEXT_BYTES, KEM_CIPHERTEXT_BYTES);
            ct_to_use = kem_ciphertext;
        }
        
        trigger_high();
        crypto_kem_dec(kem_shared_secret, ct_to_use, kem_secret_key);
        trigger_low();
        
        // Return shared secret
        simpleserial_put('s', KEM_SHARED_SECRET_BYTES, kem_shared_secret);
        return 0x00;
    }
    
    if (len != req_len) {
        return SS_ERR_LEN;
    }
    
    return 0x00;
}
#endif

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    // Clear all key storage
    memset(kem_public_key, 0, KEM_PUBLIC_KEY_BYTES);
    memset(kem_secret_key, 0, KEM_SECRET_KEY_BYTES);
    memset(kem_ciphertext, 0, KEM_CIPHERTEXT_BYTES);
    memset(kem_shared_secret, 0, KEM_SHARED_SECRET_BYTES);

    /* Uncomment this to get a HELLO message for debug */
    // putch('h');
    // putch('e');
    // putch('l');
    // putch('l');
    // putch('o');
    // putch('\n');

    simpleserial_init();
    
#if SS_VER == SS_VER_2_1
    // Unified KEM command (cmd=0x02 to avoid conflict with AES 0x01)
    simpleserial_addcmd(0x02, KEM_PUBLIC_KEY_BYTES + KEM_SECRET_KEY_BYTES + KEM_CIPHERTEXT_BYTES, kem);
#else
    // Individual commands for SS_VER_1
    simpleserial_addcmd('k', KEM_PUBLIC_KEY_BYTES, kem_keygen);
    simpleserial_addcmd('e', KEM_PUBLIC_KEY_BYTES, kem_encaps);
    simpleserial_addcmd('d', KEM_CIPHERTEXT_BYTES, kem_decaps);
    simpleserial_addcmd('p', KEM_PUBLIC_KEY_BYTES, get_pk);
    simpleserial_addcmd('s', KEM_SECRET_KEY_BYTES, get_sk);
    simpleserial_addcmd('x', 0, reset);
#endif
    
    while(1)
        simpleserial_get();
}
