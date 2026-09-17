#ifndef CRYPTO_PRIMITIVES_TEMPLATE_CIPHER_H
#define CRYPTO_PRIMITIVES_TEMPLATE_CIPHER_H

#include "cipher.h"

typedef struct {
    uint8_t round_keys[16];
} template_cipher_ctx;

extern const block_cipher template_block_cipher;

void template_expand_key(void *ctx, const uint8_t *master_key);
void template_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void template_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void template_clear(void *ctx);

#endif
