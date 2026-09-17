#ifndef CRYPTO_PRIMITIVES_HIGHT_LUT_H
#define CRYPTO_PRIMITIVES_HIGHT_LUT_H

#include "cipher.h"

#include <stdint.h>

typedef struct {
    uint8_t whitening_keys[8];
    uint8_t round_keys[128];
} hight_lut_ctx;

extern const block_cipher hight_lut_block_cipher;

void hight_lut_expand_key(void *ctx, const uint8_t *master_key);
void hight_lut_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void hight_lut_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void hight_lut_clear(void *ctx);

#endif
