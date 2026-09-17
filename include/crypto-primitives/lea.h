#ifndef CRYPTO_PRIMITIVES_LEA_H
#define CRYPTO_PRIMITIVES_LEA_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    uint32_t round_keys[192];
} lea_ctx;

extern const block_cipher lea128_block_cipher;
extern const block_cipher lea192_block_cipher;
extern const block_cipher lea256_block_cipher;

void lea128_expand_key(void *ctx, const uint8_t *master_key);
void lea192_expand_key(void *ctx, const uint8_t *master_key);
void lea256_expand_key(void *ctx, const uint8_t *master_key);
void lea_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void lea_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void lea_clear(void *ctx);

#endif
