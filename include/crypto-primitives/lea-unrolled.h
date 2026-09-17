#ifndef CRYPTO_PRIMITIVES_LEA_UNROLLED_H
#define CRYPTO_PRIMITIVES_LEA_UNROLLED_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    uint8_t round_keys[768];
} lea_unrolled_ctx;

extern const block_cipher lea128_unrolled_block_cipher;
extern const block_cipher lea192_unrolled_block_cipher;
extern const block_cipher lea256_unrolled_block_cipher;

void lea128_unrolled_expand_key(void *ctx, const uint8_t *master_key);
void lea192_unrolled_expand_key(void *ctx, const uint8_t *master_key);
void lea256_unrolled_expand_key(void *ctx, const uint8_t *master_key);
void lea_unrolled_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void lea_unrolled_decrypt(void *ctx, uint8_t *out, const uint8_t *in);

#endif
