#ifndef CRYPTO_PRIMITIVES_AES_H
#define CRYPTO_PRIMITIVES_AES_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    uint32_t round_keys[60];
} aes_ctx;

extern const block_cipher aes128_block_cipher;
extern const block_cipher aes192_block_cipher;
extern const block_cipher aes256_block_cipher;

void aes128_expand_key(void *ctx, const uint8_t *master_key);
void aes192_expand_key(void *ctx, const uint8_t *master_key);
void aes256_expand_key(void *ctx, const uint8_t *master_key);
void aes_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void aes_decrypt(void *ctx, uint8_t *out, const uint8_t *in);

#endif
