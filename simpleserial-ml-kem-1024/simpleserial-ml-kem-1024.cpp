/*
    ML-KEM-1024 (Kyber1024) SimpleSerial Target (C++ version)
    Adapted for ChipWhisperer
    
    This implements the ML-KEM-1024 key encapsulation mechanism
    with SimpleSerial v1.x and v2.1 support.
    
    Protocol (SS 2.1):
    - Command 0x01: Key generation (keypair)
      - scmd 0x01: Generate keypair, return first chunk of pk
      - scmd 0x02: Get pk chunk (buf[0]=chunk_num, returns 240 bytes)
      - scmd 0x04: Get sk chunk (buf[0]=chunk_num, returns 240 bytes)
      - scmd 0x10: Load pk chunk (buf[0]=chunk_num + 239 bytes data)
      - scmd 0x20: Load sk chunk (buf[0]=chunk_num + 239 bytes data)
    
    - Command 0x02: Encapsulation
      - scmd 0x01: Encapsulate using stored pk, return first chunk of ct
      - scmd 0x02: Get ct chunk (buf[0]=chunk_num, returns 240 bytes)
      - scmd 0x10: Load pk chunk for encapsulation
    
    - Command 0x03: Decapsulation
      - scmd 0x01: Decapsulate using stored sk + ct, return ss (32 bytes)
      - scmd 0x10: Load ct chunk for decapsulation
    
    For SS v1.x:
    - 'k': Generate keypair
    - 'p': Get public key (returns all 1568 bytes)
    - 's': Get secret key (returns all 3168 bytes)
    - 'e': Encapsulate (uses stored pk, returns ct)
    - 'd': Decapsulate (uses stored sk + received ct, returns ss)
    - 'x': Reset/clear stored keys
*/

extern "C" {
    #include "hal.h"
    #include "simpleserial.h"
    #include "api.h"
    #include "params.h"
}

#include <stdint.h>
#include <string.h>

/* ML-KEM-1024 sizes */
#define MLKEM_PKBYTES  1568  /* Public key bytes */
#define MLKEM_SKBYTES  3168  /* Secret key bytes */
#define MLKEM_CTBYTES  1568  /* Ciphertext bytes */
#define MLKEM_SSBYTES  32    /* Shared secret bytes */

#define CHUNK_SIZE 240       /* Max chunk size for SS 2.1 */

/* Storage for keys and ciphertext */
static uint8_t pk[MLKEM_PKBYTES];
static uint8_t sk[MLKEM_SKBYTES];
static uint8_t ct[MLKEM_CTBYTES];
static uint8_t ss[MLKEM_SSBYTES];

static int pk_loaded = 0;
static int sk_loaded = 0;
static int ct_loaded = 0;

/* Maximum number of chunks */
#define MAX_PK_CHUNKS  ((MLKEM_PKBYTES + CHUNK_SIZE - 1) / CHUNK_SIZE)  /* 7 */
#define MAX_SK_CHUNKS  ((MLKEM_SKBYTES + CHUNK_SIZE - 1) / CHUNK_SIZE)  /* 14 */
#define MAX_CT_CHUNKS  ((MLKEM_CTBYTES + CHUNK_SIZE - 1) / CHUNK_SIZE)  /* 7 */

/*******************************************/
/* SimpleSerial v2.1 Command Handler       */
/*******************************************/

#if SS_VER == SS_VER_2_1

/* Command 0x01: Key Generation */
uint8_t mlkem_keygen(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t chunk;
    uint16_t offset;
    uint8_t num_bytes;
    
    if (scmd == 0x01) {
        /* Generate keypair */
        trigger_high();
        crypto_kem_keypair(pk, sk);
        trigger_low();
        
        pk_loaded = 1;
        sk_loaded = 1;
        
        /* Return first chunk of pk (240 bytes or less) */
        num_bytes = (MLKEM_PKBYTES > CHUNK_SIZE) ? CHUNK_SIZE : MLKEM_PKBYTES;
        memcpy(buf, pk, num_bytes);
        return 0;
    }
    else if (scmd == 0x02) {
        /* Get pk chunk */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_PKBYTES) {
            /* Return empty if beyond data */
            return 0;
        }
        
        num_bytes = ((MLKEM_PKBYTES - offset) > CHUNK_SIZE) ? CHUNK_SIZE : (MLKEM_PKBYTES - offset);
        memcpy(buf, pk + offset, num_bytes);
        return 0;
    }
    else if (scmd == 0x04) {
        /* Get sk chunk */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_SKBYTES) {
            return 0;
        }
        
        num_bytes = ((MLKEM_SKBYTES - offset) > CHUNK_SIZE) ? CHUNK_SIZE : (MLKEM_SKBYTES - offset);
        memcpy(buf, sk + offset, num_bytes);
        return 0;
    }
    else if (scmd == 0x10) {
        /* Load pk chunk */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_PKBYTES) {
            return SS_ERR_LEN;
        }
        
        /* Copy data (len-1 bytes of actual data) */
        num_bytes = len - 1;
        if (num_bytes + offset > MLKEM_PKBYTES) {
            num_bytes = MLKEM_PKBYTES - offset;
        }
        memcpy(pk + offset, buf + 1, num_bytes);
        
        /* Check if full pk loaded */
        if (offset + num_bytes >= MLKEM_PKBYTES) {
            pk_loaded = 1;
        }
        return 0;
    }
    else if (scmd == 0x20) {
        /* Load sk chunk */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_SKBYTES) {
            return SS_ERR_LEN;
        }
        
        num_bytes = len - 1;
        if (num_bytes + offset > MLKEM_SKBYTES) {
            num_bytes = MLKEM_SKBYTES - offset;
        }
        memcpy(sk + offset, buf + 1, num_bytes);
        
        /* Check if full sk loaded */
        if (offset + num_bytes >= MLKEM_SKBYTES) {
            sk_loaded = 1;
        }
        return 0;
    }
    
    return SS_ERR_CMD;
}

/* Command 0x02: Encapsulation */
uint8_t mlkem_encaps(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t chunk;
    uint16_t offset;
    uint8_t num_bytes;
    
    if (!pk_loaded) {
        /* No pk loaded, cannot encapsulate */
        return SS_ERR_CMD;
    }
    
    if (scmd == 0x01) {
        /* Perform encapsulation */
        trigger_high();
        crypto_kem_enc(ct, ss, pk);
        trigger_low();
        
        ct_loaded = 1;
        
        /* Return first chunk of ct */
        num_bytes = (MLKEM_CTBYTES > CHUNK_SIZE) ? CHUNK_SIZE : MLKEM_CTBYTES;
        memcpy(buf, ct, num_bytes);
        return 0;
    }
    else if (scmd == 0x02) {
        /* Get ct chunk */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_CTBYTES) {
            return 0;
        }
        
        num_bytes = ((MLKEM_CTBYTES - offset) > CHUNK_SIZE) ? CHUNK_SIZE : (MLKEM_CTBYTES - offset);
        memcpy(buf, ct + offset, num_bytes);
        return 0;
    }
    else if (scmd == 0x10) {
        /* Load pk chunk for encapsulation */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_PKBYTES) {
            return SS_ERR_LEN;
        }
        
        num_bytes = len - 1;
        if (num_bytes + offset > MLKEM_PKBYTES) {
            num_bytes = MLKEM_PKBYTES - offset;
        }
        memcpy(pk + offset, buf + 1, num_bytes);
        
        if (offset + num_bytes >= MLKEM_PKBYTES) {
            pk_loaded = 1;
        }
        return 0;
    }
    
    return SS_ERR_CMD;
}

/* Command 0x03: Decapsulation */
uint8_t mlkem_decaps(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t chunk;
    uint16_t offset;
    uint8_t num_bytes;
    
    if (!sk_loaded) {
        /* No sk loaded, cannot decapsulate */
        return SS_ERR_CMD;
    }
    
    if (scmd == 0x01) {
        /* Need ct loaded */
        if (!ct_loaded) {
            return SS_ERR_CMD;
        }
        
        /* Perform decapsulation */
        trigger_high();
        crypto_kem_dec(ss, ct, sk);
        trigger_low();
        
        /* Return shared secret (32 bytes) */
        memcpy(buf, ss, MLKEM_SSBYTES);
        return 0;
    }
    else if (scmd == 0x10) {
        /* Load ct chunk for decapsulation */
        if (len < 1) return SS_ERR_LEN;
        chunk = buf[0];
        offset = chunk * CHUNK_SIZE;
        
        if (offset >= MLKEM_CTBYTES) {
            return SS_ERR_LEN;
        }
        
        num_bytes = len - 1;
        if (num_bytes + offset > MLKEM_CTBYTES) {
            num_bytes = MLKEM_CTBYTES - offset;
        }
        memcpy(ct + offset, buf + 1, num_bytes);
        
        if (offset + num_bytes >= MLKEM_CTBYTES) {
            ct_loaded = 1;
        }
        return 0;
    }
    
    return SS_ERR_CMD;
}

/* Reset command for SS 2.1 */
uint8_t mlkem_reset(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    pk_loaded = 0;
    sk_loaded = 0;
    ct_loaded = 0;
    return 0;
}

#endif /* SS_VER_2_1 */

/*******************************************/
/* SimpleSerial v1.x Command Handlers      */
/*******************************************/

#ifndef SS_VER_2_1

/* Generate keypair - 'k' command */
uint8_t get_key(uint8_t* k, uint8_t len)
{
    trigger_high();
    crypto_kem_keypair(pk, sk);
    trigger_low();
    
    pk_loaded = 1;
    sk_loaded = 1;
    
    /* Return public key */
    simpleserial_put('p', MLKEM_PKBYTES, pk);
    return 0;
}

/* Get public key - 'p' command */
uint8_t get_pk(uint8_t* p, uint8_t len)
{
    if (!pk_loaded) {
        return 0x01; /* Error: no pk loaded */
    }
    simpleserial_put('p', MLKEM_PKBYTES, pk);
    return 0;
}

/* Get secret key - 's' command */
uint8_t get_sk(uint8_t* s, uint8_t len)
{
    if (!sk_loaded) {
        return 0x01; /* Error: no sk loaded */
    }
    simpleserial_put('s', MLKEM_SKBYTES, sk);
    return 0;
}

/* Load public key - 'l' command (variable length) */
uint8_t load_pk(uint8_t* data, uint8_t len)
{
    uint16_t i;
    uint8_t chunk;
    
    if (len < 2) return 0x01;
    
    chunk = data[0];
    
    /* Offset = chunk * 240, but limit to actual data size */
    uint16_t offset = chunk * 240;
    if (offset >= MLKEM_PKBYTES) {
        return 0x02; /* Error: offset too large */
    }
    
    /* Copy data (remaining bytes after chunk number) */
    for (i = 1; i < len && (offset + i - 1) < MLKEM_PKBYTES; i++) {
        pk[offset + i - 1] = data[i];
    }
    
    /* Check if complete */
    if (offset + len - 1 >= MLKEM_PKBYTES) {
        pk_loaded = 1;
    }
    
    return 0;
}

/* Load secret key - 'm' command (variable length) */
uint8_t load_sk(uint8_t* data, uint8_t len)
{
    uint16_t i;
    uint8_t chunk;
    
    if (len < 2) return 0x01;
    
    chunk = data[0];
    
    uint16_t offset = chunk * 240;
    if (offset >= MLKEM_SKBYTES) {
        return 0x02;
    }
    
    for (i = 1; i < len && (offset + i - 1) < MLKEM_SKBYTES; i++) {
        sk[offset + i - 1] = data[i];
    }
    
    if (offset + len - 1 >= MLKEM_SKBYTES) {
        sk_loaded = 1;
    }
    
    return 0;
}

/* Encapsulate - 'e' command */
uint8_t encaps(uint8_t* ct_in, uint8_t len)
{
    uint16_t i;
    uint8_t chunk;
    
    if (!pk_loaded) {
        return 0x01; /* Error: no pk loaded */
    }
    
    /* If data provided, treat as pk to load */
    if (len > 0) {
        chunk = ct_in[0];
        uint16_t offset = chunk * 240;
        
        if (offset < MLKEM_PKBYTES) {
            for (i = 1; i < len && (offset + i - 1) < MLKEM_PKBYTES; i++) {
                pk[offset + i - 1] = ct_in[i];
            }
            if (offset + len - 1 >= MLKEM_PKBYTES) {
                pk_loaded = 1;
            }
            return 0x00;
        }
    }
    
    /* Perform encapsulation */
    trigger_high();
    crypto_kem_enc(ct, ss, pk);
    trigger_low();
    
    ct_loaded = 1;
    
    /* Return ciphertext */
    simpleserial_put('c', MLKEM_CTBYTES, ct);
    return 0;
}

/* Load ciphertext for decapsulation - 'c' command (variable length) */
uint8_t load_ct(uint8_t* data, uint8_t len)
{
    uint16_t i;
    uint8_t chunk;
    
    if (len < 2) return 0x01;
    
    chunk = data[0];
    
    uint16_t offset = chunk * 240;
    if (offset >= MLKEM_CTBYTES) {
        return 0x02;
    }
    
    for (i = 1; i < len && (offset + i - 1) < MLKEM_CTBYTES; i++) {
        ct[offset + i - 1] = data[i];
    }
    
    if (offset + len - 1 >= MLKEM_CTBYTES) {
        ct_loaded = 1;
    }
    
    return 0;
}

/* Decapsulate - 'd' command */
uint8_t decaps(uint8_t* data, uint8_t len)
{
    uint16_t i;
    uint8_t chunk;
    
    if (!sk_loaded) {
        return 0x01; /* Error: no sk loaded */
    }
    
    /* If data provided, treat as ct to load */
    if (len > 0) {
        chunk = data[0];
        uint16_t offset = chunk * 240;
        
        if (offset < MLKEM_CTBYTES) {
            for (i = 1; i < len && (offset + i - 1) < MLKEM_CTBYTES; i++) {
                ct[offset + i - 1] = data[i];
            }
            if (offset + len - 1 >= MLKEM_CTBYTES) {
                ct_loaded = 1;
            }
            
            if (!ct_loaded) {
                return 0x00; /* ct not complete yet */
            }
        }
    }
    
    if (!ct_loaded) {
        return 0x02; /* Error: no ct loaded */
    }
    
    /* Perform decapsulation */
    trigger_high();
    crypto_kem_dec(ss, ct, sk);
    trigger_low();
    
    /* Return shared secret */
    simpleserial_put('s', MLKEM_SSBYTES, ss);
    return 0;
}

/* Reset - 'x' command */
uint8_t reset(uint8_t* x, uint8_t len)
{
    pk_loaded = 0;
    sk_loaded = 0;
    ct_loaded = 0;
    return 0;
}

#endif /* !SS_VER_2_1 */

/*******************************************/
/* Main Function                           */
/*******************************************/

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();
    
    simpleserial_init();
    
#if SS_VER == SS_VER_2_1
    /* Register commands for SimpleSerial 2.1 */
    /* 0x01 = keygen */
    simpleserial_addcmd(0x01, CHUNK_SIZE, mlkem_keygen);
    /* 0x02 = encapsulate */
    simpleserial_addcmd(0x02, CHUNK_SIZE, mlkem_encaps);
    /* 0x03 = decapsulate */
    simpleserial_addcmd(0x03, CHUNK_SIZE, mlkem_decaps);
    /* 0x00 = reset */
    simpleserial_addcmd(0x00, 0, mlkem_reset);
#else
    /* Register commands for SimpleSerial 1.x */
    /* 'k' = generate keypair */
    simpleserial_addcmd('k', 0, get_key);
    /* 'p' = get public key */
    simpleserial_addcmd('p', 0, get_pk);
    /* 's' = get secret key */
    simpleserial_addcmd('s', 0, get_sk);
    /* 'l' = load public key (variable length) */
    simpleserial_addcmd_flags('l', 241, load_pk, CMD_FLAG_LEN);
    /* 'm' = load secret key (variable length) */
    simpleserial_addcmd_flags('m', 241, load_sk, CMD_FLAG_LEN);
    /* 'e' = encapsulate */
    simpleserial_addcmd_flags('e', 241, encaps, CMD_FLAG_LEN);
    /* 'c' = load ciphertext (variable length) */
    simpleserial_addcmd_flags('c', 241, load_ct, CMD_FLAG_LEN);
    /* 'd' = decapsulate */
    simpleserial_addcmd_flags('d', 241, decaps, CMD_FLAG_LEN);
    /* 'x' = reset */
    simpleserial_addcmd('x', 0, reset);
#endif
    
    while(1)
        simpleserial_get();
    
    return 0;
}
