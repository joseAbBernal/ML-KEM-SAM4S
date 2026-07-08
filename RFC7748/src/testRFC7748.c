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

/* Helper: print 32 bytes as hex in the same byte-order as RFC 7748 vectors. */
static void print_hex(const char *label, const uint8_t buf[32])
{
    int i;
    printf("%s", label);
    for (i = 0; i < 32; i++)
        printf("%02x", buf[i]);
    printf("\n");
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
    int passed = 0, total = 2;

    printf("=== RFC 7748 X25519 test suite (WORD_BITS=%d) ===\n\n", WORD_BITS);

    passed += test_rfc7748_vector_a();
    printf("\n");
    passed += test_rfc7748_ecdh();
    printf("\n");

    printf("Results: %d/%d tests passed.\n", passed, total);
    return (passed == total) ? 0 : 1;
}
