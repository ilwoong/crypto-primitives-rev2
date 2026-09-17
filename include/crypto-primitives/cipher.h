#ifndef CRYPTO_PRIMITIVES_CIPHER_H
#define CRYPTO_PRIMITIVES_CIPHER_H

#include <stddef.h>
#include <stdint.h>

#include "secure-zero.h"

typedef void (*expand_key_fn)(void *ctx, const uint8_t *master_key);
typedef void (*crypt_block_fn)(void *ctx, uint8_t *out, const uint8_t *in);
typedef void (*clear_ctx_fn)(void *ctx);

typedef struct {
    size_t block_size;
    size_t key_size;
    expand_key_fn expand_key;
    crypt_block_fn encrypt;
    crypt_block_fn decrypt;
    clear_ctx_fn clear;
} block_cipher;

#endif
