#include "api.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void print_hex(const char *name, const uint8_t *data, size_t len) {
    printf("%s[%zu] = {\n", name, len);
    for (size_t i = 0; i < len; i++) {
        printf("0x%02x", data[i]);
        if (i < len - 1) printf(", ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n};\n\n");
}

int main(void) {
    uint8_t pk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_SECRETKEYBYTES];
    uint8_t ct[PQCLEAN_MLKEM512_CLEAN_CRYPTO_CIPHERTEXTBYTES];
    uint8_t ct_tampered[PQCLEAN_MLKEM512_CLEAN_CRYPTO_CIPHERTEXTBYTES];
    uint8_t ss[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];
    uint8_t ss_tampered[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];
    uint8_t ss2[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];

    printf("\n========================================\n");
    printf("ML-KEM-512 Test with FAIL value analysis\n");
    printf("========================================\n\n");

    printf("1. Generating keypair...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_keypair(pk, sk);
    printf("   Public key:  %zu bytes\n", sizeof(pk));
    printf("   Secret key: %zu bytes\n", sizeof(sk));

    printf("\n2. Encrypting...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_enc(ct, ss, pk);
    printf("   Ciphertext: %zu bytes\n", sizeof(ct));
    printf("   Shared secret (enc): %zu bytes\n", sizeof(ss));

    printf("\n3. Decrypting with VALID ciphertext (fail = 0)...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_dec(ss2, ct, sk);
    printf("   Shared secret (dec): %zu bytes\n", sizeof(ss2));

    printf("\n4. Verifying shared secret match (valid ct)...\n");
    int match = 1;
    for (size_t i = 0; i < PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES; i++) {
        if (ss[i] != ss2[i]) {
            match = 0;
            break;
        }
    }
    if (match) {
        printf("   ✓ SUCCESS: Shared secrets match!\n");
        printf("   ✓ fail = 0 (ciphertext is valid)\n");
    } else {
        printf("   ✗ FAILURE: Shared secrets do not match!\n");
    }

    printf("\n5. Creating TAMPERED ciphertext (fail = 1)...\n");
    memcpy(ct_tampered, ct, PQCLEAN_MLKEM512_CLEAN_CRYPTO_CIPHERTEXTBYTES);
    ct_tampered[0] ^= 0xFF;  /* Flip all bits in first byte */
    printf("   Modified first byte: 0x%02x → 0x%02x\n", ct[0], ct_tampered[0]);
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_dec(ss_tampered, ct_tampered, sk);
    printf("   fail = 1 (ciphertext is corrupted)\n");
    printf("   Shared secret (tampered): %zu bytes\n", sizeof(ss_tampered));

    printf("\n===== TEST VECTORS FOR ARM SAM4S =====\n");
    printf("\n--- CASE 1: fail = 0 (Valid ciphertext for cmov with condition TRUE) ---\n");
    printf("When this ciphertext is decapsulated, cmov() copies the TRUE secret (kr)\n");
    printf("because verify(ct, cmp) returns 0\n\n");
    print_hex("pk", pk, sizeof(pk));
    print_hex("sk", sk, sizeof(sk));
    print_hex("ct", ct, sizeof(ct));
    printf("/* Expected shared secret (from encapsulation): */\n");
    print_hex("ss_expected", ss, sizeof(ss));

    printf("\n--- CASE 2: fail = 1 (Tampered ciphertext for cmov with condition FALSE) ---\n");
    printf("When this ciphertext is decapsulated, cmov() does NOT copy (keeps rejection key)\n");
    printf("because verify(ct_tampered, cmp) returns 1\n\n");
    print_hex("pk", pk, sizeof(pk));
    print_hex("sk", sk, sizeof(sk));
    print_hex("ct_tampered", ct_tampered, sizeof(ct_tampered));
    printf("/* Shared secret will be rejection key (NOT from encapsulation): */\n");
    print_hex("ss_rejection", ss_tampered, sizeof(ss_tampered));

    printf("\n===== COMPARISON =====\n");
    printf("Valid case shared secret:    "); 
    for (int i = 0; i < 16; i++) printf("%02x", ss[i]);
    printf("...\n");
    printf("Tampered case shared secret: "); 
    for (int i = 0; i < 16; i++) printf("%02x", ss_tampered[i]);
    printf("...\n");
    printf("Are they equal? %s\n", memcmp(ss, ss_tampered, 32) == 0 ? "YES (ERROR!)" : "NO (expected - different keys)");

    return 0;
}