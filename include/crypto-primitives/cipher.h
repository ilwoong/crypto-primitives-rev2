#ifndef CRYPTO_PRIMITIVES_CIPHER_H
#define CRYPTO_PRIMITIVES_CIPHER_H

#include <stdint.h>

typedef void (*expand_key_fn)(void *ctx, const uint8_t *master_key);
typedef void (*crypt_block_fn)(void *ctx, uint8_t *out, const uint8_t *in);

typedef struct {
    expand_key_fn expand_key;
    crypt_block_fn encrypt;
    crypt_block_fn decrypt;
} BlockCipher;

#endif
