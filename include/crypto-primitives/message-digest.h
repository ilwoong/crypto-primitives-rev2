#ifndef CRYPTO_PRIMITIVES_MESSAGE_DIGEST_H
#define CRYPTO_PRIMITIVES_MESSAGE_DIGEST_H

#include <stddef.h>
#include <stdint.h>

#include "secure-zero.h"

typedef void (*message_digest_init_fn)(void *ctx);
typedef void (*message_digest_update_fn)(void *ctx, const uint8_t *in, size_t len);
typedef void (*message_digest_final_fn)(void *ctx, uint8_t *out);

typedef struct {
    size_t digest_size;
    size_t block_size;
    message_digest_init_fn init;
    message_digest_update_fn update;
    message_digest_final_fn final;
} message_digest;

#endif
