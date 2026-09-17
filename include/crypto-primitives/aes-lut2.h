#ifndef CRYPTO_PRIMITIVES_AES_LUT2_H
#define CRYPTO_PRIMITIVES_AES_LUT2_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    uint8_t round_keys[240];
} aes_lut2_ctx;

extern const block_cipher aes128_lut2_block_cipher;
extern const block_cipher aes192_lut2_block_cipher;
extern const block_cipher aes256_lut2_block_cipher;

void aes128_lut2_expand_key(void *ctx, const uint8_t *master_key);
void aes192_lut2_expand_key(void *ctx, const uint8_t *master_key);
void aes256_lut2_expand_key(void *ctx, const uint8_t *master_key);
void aes_lut2_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void aes_lut2_decrypt(void *ctx, uint8_t *out, const uint8_t *in);

#endif
