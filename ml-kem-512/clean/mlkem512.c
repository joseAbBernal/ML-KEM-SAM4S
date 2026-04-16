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
    uint8_t ss[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];
    uint8_t ss2[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];

    printf("ML-KEM-512 Test\n");
    printf("================\n\n");

    printf("1. Generating keypair...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_keypair(pk, sk);
    printf("   Public key:  %zu bytes\n", sizeof(pk));
    printf("   Secret key: %zu bytes\n", sizeof(sk));

    printf("\n2. Encrypting...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_enc(ct, ss, pk);
    printf("   Ciphertext: %zu bytes\n", sizeof(ct));
    printf("   Shared secret (enc): %zu bytes\n", sizeof(ss));

    printf("\n3. Decrypting...\n");
    PQCLEAN_MLKEM512_CLEAN_crypto_kem_dec(ss2, ct, sk);
    printf("   Shared secret (dec): %zu bytes\n", sizeof(ss2));

    printf("\n4. Verifying shared secret match...\n");
    int match = 1;
    for (size_t i = 0; i < PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES; i++) {
        if (ss[i] != ss2[i]) {
            match = 0;
            break;
        }
    }
    if (match) {
        printf("   SUCCESS: Shared secrets match!\n");
    } else {
        printf("   FAILURE: Shared secrets do not match!\n");
    }

    printf("\n===== HEX DUMPS FOR ARM SAM4S =====\n\n");
    print_hex("pk", pk, sizeof(pk));
    print_hex("sk", sk, sizeof(sk));
    print_hex("ct", ct, sizeof(ct));
    print_hex("ss", ss, sizeof(ss));
    print_hex("ss2", ss2, sizeof(ss2));

    return 0;
}