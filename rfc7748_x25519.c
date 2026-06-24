#include <openssl/evp.h>
#include <openssl/rand.h>

#include <stdio.h>
#include <string.h>

#define X25519_KEY_LEN 32

static int hex_to_bytes(const char *hex, unsigned char *out, size_t out_len) {
  size_t i;
  if (strlen(hex) != out_len * 2) {
    return 0;
  }

  for (i = 0; i < out_len; ++i) {
    unsigned int v;
    if (sscanf(&hex[i * 2], "%2x", &v) != 1) {
      return 0;
    }
    out[i] = (unsigned char)v;
  }
  return 1;
}

static void print_hex(const char *label, const unsigned char *buf, size_t len) {
  size_t i;
  printf("%s", label);
  for (i = 0; i < len; ++i) {
    printf("%02x", buf[i]);
  }
  printf("\n");
}

/*
 * RFC7748 X25519 shared-secret derivation:
 * out = X25519(private_scalar, peer_public_u_coordinate)
 */
static int x25519_derive(const unsigned char private_scalar[X25519_KEY_LEN],
                         const unsigned char peer_public[X25519_KEY_LEN],
                         unsigned char out_shared[X25519_KEY_LEN]) {
  int ok = 0;
  EVP_PKEY *self = NULL;
  EVP_PKEY *peer = NULL;
  EVP_PKEY_CTX *ctx = NULL;
  size_t out_len = X25519_KEY_LEN;

  self = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, private_scalar,
                                      X25519_KEY_LEN);
  if (self == NULL) {
    goto cleanup;
  }

  peer = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, peer_public,
                                     X25519_KEY_LEN);
  if (peer == NULL) {
    goto cleanup;
  }

  ctx = EVP_PKEY_CTX_new(self, NULL);
  if (ctx == NULL) {
    goto cleanup;
  }

  if (EVP_PKEY_derive_init(ctx) <= 0) {
    goto cleanup;
  }

  if (EVP_PKEY_derive_set_peer(ctx, peer) <= 0) {
    goto cleanup;
  }

  if (EVP_PKEY_derive(ctx, out_shared, &out_len) <= 0) {
    goto cleanup;
  }

  ok = (out_len == X25519_KEY_LEN);

cleanup:
  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(peer);
  EVP_PKEY_free(self);
  return ok;
}

static int run_rfc7748_test_vector(void) {
  static const char *alice_private_hex =
      "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a";
  static const char *alice_public_hex =
      "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";
  static const char *bob_private_hex =
      "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb";
  static const char *bob_public_hex =
      "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";
  static const char *expected_shared_hex =
      "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742";

  unsigned char alice_private[X25519_KEY_LEN];
  unsigned char alice_public[X25519_KEY_LEN];
  unsigned char bob_private[X25519_KEY_LEN];
  unsigned char bob_public[X25519_KEY_LEN];
  unsigned char expected_shared[X25519_KEY_LEN];
  unsigned char shared_ab[X25519_KEY_LEN];
  unsigned char shared_ba[X25519_KEY_LEN];

  if (!hex_to_bytes(alice_private_hex, alice_private, X25519_KEY_LEN) ||
      !hex_to_bytes(alice_public_hex, alice_public, X25519_KEY_LEN) ||
      !hex_to_bytes(bob_private_hex, bob_private, X25519_KEY_LEN) ||
      !hex_to_bytes(bob_public_hex, bob_public, X25519_KEY_LEN) ||
      !hex_to_bytes(expected_shared_hex, expected_shared, X25519_KEY_LEN)) {
    fprintf(stderr, "Error convirtiendo vectores hex.\n");
    return 0;
  }

  if (!x25519_derive(alice_private, bob_public, shared_ab)) {
    fprintf(stderr, "Fallo derivando secreto Alice->Bob.\n");
    return 0;
  }

  if (!x25519_derive(bob_private, alice_public, shared_ba)) {
    fprintf(stderr, "Fallo derivando secreto Bob->Alice.\n");
    return 0;
  }

  print_hex("Shared Alice->Bob: ", shared_ab, X25519_KEY_LEN);
  print_hex("Shared Bob->Alice: ", shared_ba, X25519_KEY_LEN);
  print_hex("Shared esperado : ", expected_shared, X25519_KEY_LEN);

  if (memcmp(shared_ab, shared_ba, X25519_KEY_LEN) != 0) {
    fprintf(stderr, "No coincide Alice/Bob.\n");
    return 0;
  }

  if (memcmp(shared_ab, expected_shared, X25519_KEY_LEN) != 0) {
    fprintf(stderr, "No coincide con el vector RFC7748.\n");
    return 0;
  }

  printf("Vector RFC7748 X25519: OK\n");
  return 1;
}

int main(void) {
  if (!run_rfc7748_test_vector()) {
    return 1;
  }

  /*
   * Demostracion opcional: generar una clave privada aleatoria y derivar
   * un secreto usando la publica de Bob del vector RFC.
   */
  {
    unsigned char my_private[X25519_KEY_LEN];
    unsigned char bob_public[X25519_KEY_LEN];
    unsigned char my_shared[X25519_KEY_LEN];
    static const char *bob_public_hex =
        "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";

    if (RAND_bytes(my_private, X25519_KEY_LEN) != 1) {
      fprintf(stderr, "No se pudo generar aleatorio.\n");
      return 1;
    }

    if (!hex_to_bytes(bob_public_hex, bob_public, X25519_KEY_LEN)) {
      fprintf(stderr, "Error parseando clave publica de Bob.\n");
      return 1;
    }

    if (!x25519_derive(my_private, bob_public, my_shared)) {
      fprintf(stderr, "Fallo en derivacion X25519 aleatoria.\n");
      return 1;
    }

    print_hex("Shared aleatorio: ", my_shared, X25519_KEY_LEN);
  }

  return 0;
}
