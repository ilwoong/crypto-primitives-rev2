#include "crypto-primitives/template-cipher.h"

#include <stddef.h>
#include <string.h>

static const size_t NUM_ROUNDS = 1;
static const size_t BLOCK_SIZE = 16;
static const size_t MASTER_KEY_SIZE = 16;

const block_cipher template_block_cipher = {
    .expand_key = template_expand_key,
    .encrypt = template_encrypt,
    .decrypt = template_decrypt,
};

void template_expand_key(void *ctx, const uint8_t *master_key)
{
    template_cipher_ctx *actual_ctx = (template_cipher_ctx *)ctx;
    memcpy(actual_ctx->round_keys, master_key, MASTER_KEY_SIZE);
}

void template_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    template_cipher_ctx *actual_ctx = (template_cipher_ctx *)ctx;
    memcpy(out, in, BLOCK_SIZE);
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        out[i] ^= actual_ctx->round_keys[i];
    }
}

void template_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    template_cipher_ctx *actual_ctx = (template_cipher_ctx *)ctx;
    memcpy(out, in, BLOCK_SIZE);
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        out[i] ^= actual_ctx->round_keys[i];
    }
}
