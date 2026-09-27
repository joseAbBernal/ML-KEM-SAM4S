/*
 * testRFC7748.c – Test vectors for the X25519 implementation (RFC 7748).
 *
 * Covers:
 *  - RFC 7748 §6.1 test vector: X25519(k, u) with the official scalar/point pair.
 *  - RFC 7748 §6.1 ECDH test: full Alice/Bob key exchange and shared-secret check.
 *
 * The expected values are the authoritative hex strings from the RFC itself.
 *
 * Build (see Makefile):
 *   make 32   or   make 64
 * Run:
 *   ./x25519test
 */

#include "types.h"
#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Helper: convert a 64-character hex string to 32 bytes.
 * RFC 7748 test vectors are given as hex with byte[0] (least significant)
 * first, which is the same order used by the X25519 byte arrays.
 * Returns 1 on success, 0 on error.
 * ---------------------------------------------------------------------- */
static int hex_to_bytes(const char *hex, uint8_t out[32])
{
    size_t i;
    if (strlen(hex) != 64) return 0;
    for (i = 0; i < 32; i++) {
        unsigned int v;
        if (sscanf(&hex[i * 2], "%2x", &v) != 1) return 0;
        out[i] = (uint8_t)v;
    }
    return 1;
}

static int hex_to_bytes56(const char *hex, uint8_t out[56])
{
    size_t i;
    if (strlen(hex) != 112) return 0;
    for (i = 0; i < 56; i++) {
        unsigned int v;
        if (sscanf(&hex[i * 2], "%2x", &v) != 1) return 0;
        out[i] = (uint8_t)v;
    }
    return 1;
}

/* Helper: print 32 bytes as hex in the same byte-order as RFC 7748 vectors. */
static void print_hex(const char *label, const uint8_t buf[32])
{
    int i;
    printf("%s", label);
    for (i = 0; i < 32; i++)
        printf("%02x", buf[i]);
    printf("\n");
}

static void print_hex56(const char *label, const uint8_t buf[56])
{
    int i;
    printf("%s", label);
    for (i = 0; i < 56; i++)
        printf("%02x", buf[i]);
    printf("\n");
}

static int test_x448_vector(void)
{
    static const char k_hex[] =
        "3d262fddf9ec8e88495266fea19a34d28882acef045104d0d1aae121"
        "700a779c984c24f8cdd78fbff44943eba368f54b29259a4f1c600ad3";
    static const char u_hex[] =
        "06fce640fa3487bfda5f6cf2d5263f8aad88334cbd07437f020f08f9"
        "814dc031ddbdc38c19c6da2583fa5429db94ada18aa7a7fb4ef8a086";
    static const char expected_hex[] =
        "ce3e4ff95a60dc6697da1db1d85e6afbdf79b50a2412d7546d5f239f"
        "e14fbaadeb445fc66a01b0779d98223961111e21766282f73dd96b6f";
    uint8_t k[56], u[56], expected[56], result[56];

    if (!hex_to_bytes56(k_hex, k) || !hex_to_bytes56(u_hex, u) ||
        !hex_to_bytes56(expected_hex, expected)) {
        fprintf(stderr, "FAIL: hex conversion error in X448 test\n");
        return 0;
    }

    X448(k, u, result);
    print_hex56("X448 result  : ", result);
    print_hex56("X448 expected: ", expected);
    if (memcmp(result, expected, 56) != 0) {
        fprintf(stderr, "FAIL: RFC 7748 X448 vector does not match.\n");
        return 0;
    }
    printf("PASS: RFC 7748 X448 scalar multiplication\n");
    return 1;
}

static int test_x448_ecdh(void)
{
    static const uint8_t base_point[56] = {5};
    static const char alice_priv_hex[] =
        "9a8f4925d1519f5775cf46b04b5800d4ee9ee8bae8bc5565d498c28d"
        "d9c9baf574a9419744897391006382a6f127ab1d9ac2d8c0a598726b";
    static const char alice_pub_hex[] =
        "9b08f7cc31b7e3e67d22d5aea121074a273bd2b83de09c63faa73d2c"
        "22c5d9bbc836647241d953d40c5b12da88120d53177f80e532c41fa0";
    static const char bob_priv_hex[] =
        "1c306a7ac2a0e2e0990b294470cba339e6453772b075811d8fad0d1d"
        "6927c120bb5ee8972b0d3e21374c9c921b09d1b0366f10b65173992d";
    static const char bob_pub_hex[] =
        "3eb7a829b0cd20f5bcfc0b599b6feccf6da4627107bdb0d4f345b430"
        "27d8b972fc3e34fb4232a13ca706dcb57aec3dae07bdc1c67bf33609";
    static const char shared_hex[] =
        "07fff4181ac6cc95ec1c16a94a0f74d12da232ce40a77552281d282b"
        "b60c0b56fd2464c335543936521c24403085d59a449a5037514a879d";
    uint8_t alice_priv[56], alice_pub_exp[56], alice_pub[56];
    uint8_t bob_priv[56], bob_pub_exp[56], bob_pub[56];
    uint8_t shared_exp[56], shared_ab[56], shared_ba[56];
    int ok = 1;

    if (!hex_to_bytes56(alice_priv_hex, alice_priv) ||
        !hex_to_bytes56(alice_pub_hex, alice_pub_exp) ||
        !hex_to_bytes56(bob_priv_hex, bob_priv) ||
        !hex_to_bytes56(bob_pub_hex, bob_pub_exp) ||
        !hex_to_bytes56(shared_hex, shared_exp)) {
        fprintf(stderr, "FAIL: hex conversion error in X448 ECDH test\n");
        return 0;
    }

    X448(alice_priv, base_point, alice_pub);
    X448(bob_priv, base_point, bob_pub);
    X448(alice_priv, bob_pub, shared_ab);
    X448(bob_priv, alice_pub, shared_ba);

    ok &= memcmp(alice_pub, alice_pub_exp, 56) == 0;
    ok &= memcmp(bob_pub, bob_pub_exp, 56) == 0;
    ok &= memcmp(shared_ab, shared_ba, 56) == 0;
    ok &= memcmp(shared_ab, shared_exp, 56) == 0;
    if (!ok) {
        fprintf(stderr, "FAIL: RFC 7748 X448 ECDH test\n");
        return 0;
    }
    printf("PASS: RFC 7748 X448 ECDH test\n");
    return 1;
}

/* -------------------------------------------------------------------------
 * Test 1 – RFC 7748 §6.1 scalar-multiplication test vector.
 *
 *   Input scalar k  (hex, big-endian as published):
 *     a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4
 *   Input u-coordinate u (hex, big-endian):
 *     e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c
 *   Expected output (hex, big-endian):
 *     c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552
 * ---------------------------------------------------------------------- */
static int test_rfc7748_vector_a(void)
{
    static const char k_hex[]   =
        "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4";
    static const char u_hex[]   =
        "e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c";
    static const char exp_hex[] =
        "c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552";

    uint8_t k[32], u[32], expected[32], result[32];

    if (!hex_to_bytes(k_hex,   k)       ||
        !hex_to_bytes(u_hex,   u)       ||
        !hex_to_bytes(exp_hex, expected)) {
        fprintf(stderr, "FAIL: hex conversion error in test A\n");
        return 0;
    }

    X25519(k, u, result);

    print_hex("Test A result  : ", result);
    print_hex("Test A expected: ", expected);

    if (memcmp(result, expected, 32) != 0) {
        fprintf(stderr, "FAIL: RFC 7748 test vector A does not match.\n");
        return 0;
    }
    printf("PASS: RFC 7748 test vector A (X25519 scalar multiplication)\n");
    return 1;
}

/* -------------------------------------------------------------------------
 * Test 2 – RFC 7748 §6.1 ECDH test vectors (Alice / Bob key exchange).
 *
 *   Alice private key  (hex, big-endian):
 *     77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a
 *   Alice public key (= X25519(alice_priv, 9)):
 *     8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a
 *   Bob private key (hex, big-endian):
 *     5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb
 *   Bob public key (= X25519(bob_priv, 9)):
 *     de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f
 *   Shared secret (= X25519(alice_priv, bob_pub) = X25519(bob_priv, alice_pub)):
 *     4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742
 * ---------------------------------------------------------------------- */
static int test_rfc7748_ecdh(void)
{
    /* Base point u = 9 encoded as a 32-byte little-endian value. */
    static const uint8_t base_point[32] = {9};

    static const char alice_priv_hex[] =
        "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a";
    static const char alice_pub_hex[]  =
        "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";
    static const char bob_priv_hex[]   =
        "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb";
    static const char bob_pub_hex[]    =
        "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";
    static const char shared_hex[]     =
        "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742";

    uint8_t alice_priv[32], alice_pub_exp[32], alice_pub_got[32];
    uint8_t bob_priv[32],   bob_pub_exp[32],   bob_pub_got[32];
    uint8_t shared_exp[32], shared_ab[32],      shared_ba[32];
    int ok = 1;

    if (!hex_to_bytes(alice_priv_hex, alice_priv)    ||
        !hex_to_bytes(alice_pub_hex,  alice_pub_exp) ||
        !hex_to_bytes(bob_priv_hex,   bob_priv)      ||
        !hex_to_bytes(bob_pub_hex,    bob_pub_exp)   ||
        !hex_to_bytes(shared_hex,     shared_exp)) {
        fprintf(stderr, "FAIL: hex conversion error in ECDH test\n");
        return 0;
    }

    /* --- Derive public keys from private keys and the base point. --- */
    X25519(alice_priv, base_point, alice_pub_got);
    X25519(bob_priv,   base_point, bob_pub_got);

    print_hex("Alice pub got : ", alice_pub_got);
    print_hex("Alice pub exp : ", alice_pub_exp);
    print_hex("Bob   pub got : ", bob_pub_got);
    print_hex("Bob   pub exp : ", bob_pub_exp);

    if (memcmp(alice_pub_got, alice_pub_exp, 32) != 0) {
        fprintf(stderr, "FAIL: Alice public key mismatch.\n");
        ok = 0;
    }
    if (memcmp(bob_pub_got, bob_pub_exp, 32) != 0) {
        fprintf(stderr, "FAIL: Bob public key mismatch.\n");
        ok = 0;
    }

    /* --- Derive shared secrets (must be equal). --- */
    X25519(alice_priv, bob_pub_got,   shared_ab);
    X25519(bob_priv,   alice_pub_got, shared_ba);

    print_hex("Shared Alice->Bob: ", shared_ab);
    print_hex("Shared Bob->Alice: ", shared_ba);
    print_hex("Shared expected  : ", shared_exp);

    if (memcmp(shared_ab, shared_ba, 32) != 0) {
        fprintf(stderr, "FAIL: Alice/Bob shared secrets differ.\n");
        ok = 0;
    }
    if (memcmp(shared_ab, shared_exp, 32) != 0) {
        fprintf(stderr, "FAIL: Shared secret does not match RFC 7748 vector.\n");
        ok = 0;
    }

    if (ok)
        printf("PASS: RFC 7748 ECDH test (Alice/Bob key exchange)\n");
    return ok;
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    int passed = 0, total = 4;

    printf("=== RFC 7748 X25519 test suite (WORD_BITS=%d) ===\n\n", WORD_BITS);

    passed += test_rfc7748_vector_a();
    printf("\n");
    passed += test_rfc7748_ecdh();
    printf("\n");
    passed += test_x448_vector();
    printf("\n");
    passed += test_x448_ecdh();

    printf("Results: %d/%d tests passed.\n", passed, total);
    return (passed == total) ? 0 : 1;
}
