#include "crypto-primitives/cham.h"

#include <string.h>

#define CHAM64_128_ROUNDS 88
#define CHAM128_128_ROUNDS 112
#define CHAM128_256_ROUNDS 120

static inline uint16_t rol16(uint16_t value, unsigned rot)
{
    return (uint16_t)((value << rot) | (value >> (16 - rot)));
}

static inline uint16_t ror16(uint16_t value, unsigned rot)
{
    return (uint16_t)((value >> rot) | (value << (16 - rot)));
}

static inline uint32_t rol32(uint32_t value, unsigned rot)
{
    return (value << rot) | (value >> (32 - rot));
}

static inline uint32_t ror32(uint32_t value, unsigned rot)
{
    return (value >> rot) | (value << (32 - rot));
}

void cham64_128_expand_key(void *ctx, const uint8_t *master_key)
{
    cham_ctx *c = (cham_ctx *)ctx;
    c->rounds = CHAM64_128_ROUNDS;
    c->num_round_keys = 16;

    uint16_t key[8];
    memcpy(key, master_key, sizeof(key));

    uint16_t rk[16];
    for (size_t i = 0; i < 8; ++i) {
        uint16_t k = key[i] ^ rol16(key[i], 1);
        rk[i] = k ^ rol16(key[i], 8);
        rk[(i + 8) ^ 1] = k ^ rol16(key[i], 11);
    }

    for (size_t i = 0; i < 16; ++i) {
        c->round_keys[i] = rk[i];
    }
}

static void cham128_expand_key_words(cham_ctx *c, const uint8_t *master_key, size_t key_words)
{
    uint32_t key[8];
    memcpy(key, master_key, key_words * sizeof(uint32_t));

    for (size_t i = 0; i < key_words; ++i) {
        uint32_t k = key[i] ^ rol32(key[i], 1);
        c->round_keys[i] = k ^ rol32(key[i], 8);
        c->round_keys[(i + key_words) ^ 1] = k ^ rol32(key[i], 11);
    }
    c->num_round_keys = 2 * key_words;
}

void cham128_128_expand_key(void *ctx, const uint8_t *master_key)
{
    cham_ctx *c = (cham_ctx *)ctx;
    c->rounds = CHAM128_128_ROUNDS;
    cham128_expand_key_words(c, master_key, 4);
}

void cham128_256_expand_key(void *ctx, const uint8_t *master_key)
{
    cham_ctx *c = (cham_ctx *)ctx;
    c->rounds = CHAM128_256_ROUNDS;
    cham128_expand_key_words(c, master_key, 8);
}

void cham64_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const cham_ctx *c = (const cham_ctx *)ctx;
    uint16_t blk[4];
    memcpy(blk, in, sizeof(blk));

    uint16_t rc = 0;
    for (size_t r = 0; r < c->rounds; r += 8) {
        const uint32_t *rk = c->round_keys + (r % c->num_round_keys);

        blk[0] = rol16((uint16_t)((blk[0] ^ rc++) + (rol16(blk[1], 1) ^ (uint16_t)rk[0])), 8);
        blk[1] = rol16((uint16_t)((blk[1] ^ rc++) + (rol16(blk[2], 8) ^ (uint16_t)rk[1])), 1);
        blk[2] = rol16((uint16_t)((blk[2] ^ rc++) + (rol16(blk[3], 1) ^ (uint16_t)rk[2])), 8);
        blk[3] = rol16((uint16_t)((blk[3] ^ rc++) + (rol16(blk[0], 8) ^ (uint16_t)rk[3])), 1);

        blk[0] = rol16((uint16_t)((blk[0] ^ rc++) + (rol16(blk[1], 1) ^ (uint16_t)rk[4])), 8);
        blk[1] = rol16((uint16_t)((blk[1] ^ rc++) + (rol16(blk[2], 8) ^ (uint16_t)rk[5])), 1);
        blk[2] = rol16((uint16_t)((blk[2] ^ rc++) + (rol16(blk[3], 1) ^ (uint16_t)rk[6])), 8);
        blk[3] = rol16((uint16_t)((blk[3] ^ rc++) + (rol16(blk[0], 8) ^ (uint16_t)rk[7])), 1);
    }

    memcpy(out, blk, sizeof(blk));
}

void cham64_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const cham_ctx *c = (const cham_ctx *)ctx;
    uint16_t blk[4];
    memcpy(blk, in, sizeof(blk));

    uint16_t rc = (uint16_t)c->rounds;
    for (size_t r = c->rounds; r > 0; r -= 8) {
        const uint32_t *rk = c->round_keys + ((r - 8) % c->num_round_keys);

        blk[3] = (uint16_t)(ror16(blk[3], 1) - (rol16(blk[0], 8) ^ (uint16_t)rk[7])) ^ --rc;
        blk[2] = (uint16_t)(ror16(blk[2], 8) - (rol16(blk[3], 1) ^ (uint16_t)rk[6])) ^ --rc;
        blk[1] = (uint16_t)(ror16(blk[1], 1) - (rol16(blk[2], 8) ^ (uint16_t)rk[5])) ^ --rc;
        blk[0] = (uint16_t)(ror16(blk[0], 8) - (rol16(blk[1], 1) ^ (uint16_t)rk[4])) ^ --rc;

        blk[3] = (uint16_t)(ror16(blk[3], 1) - (rol16(blk[0], 8) ^ (uint16_t)rk[3])) ^ --rc;
        blk[2] = (uint16_t)(ror16(blk[2], 8) - (rol16(blk[3], 1) ^ (uint16_t)rk[2])) ^ --rc;
        blk[1] = (uint16_t)(ror16(blk[1], 1) - (rol16(blk[2], 8) ^ (uint16_t)rk[1])) ^ --rc;
        blk[0] = (uint16_t)(ror16(blk[0], 8) - (rol16(blk[1], 1) ^ (uint16_t)rk[0])) ^ --rc;
    }

    memcpy(out, blk, sizeof(blk));
}

void cham128_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const cham_ctx *c = (const cham_ctx *)ctx;
    uint32_t blk[4];
    memcpy(blk, in, sizeof(blk));

    uint32_t rc = 0;
    for (size_t r = 0; r < c->rounds; r += 8) {
        const uint32_t *rk = c->round_keys + (r % c->num_round_keys);

        blk[0] = rol32((blk[0] ^ rc++) + (rol32(blk[1], 1) ^ rk[0]), 8);
        blk[1] = rol32((blk[1] ^ rc++) + (rol32(blk[2], 8) ^ rk[1]), 1);
        blk[2] = rol32((blk[2] ^ rc++) + (rol32(blk[3], 1) ^ rk[2]), 8);
        blk[3] = rol32((blk[3] ^ rc++) + (rol32(blk[0], 8) ^ rk[3]), 1);

        blk[0] = rol32((blk[0] ^ rc++) + (rol32(blk[1], 1) ^ rk[4]), 8);
        blk[1] = rol32((blk[1] ^ rc++) + (rol32(blk[2], 8) ^ rk[5]), 1);
        blk[2] = rol32((blk[2] ^ rc++) + (rol32(blk[3], 1) ^ rk[6]), 8);
        blk[3] = rol32((blk[3] ^ rc++) + (rol32(blk[0], 8) ^ rk[7]), 1);
    }

    memcpy(out, blk, sizeof(blk));
}

void cham128_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const cham_ctx *c = (const cham_ctx *)ctx;
    uint32_t blk[4];
    memcpy(blk, in, sizeof(blk));

    uint32_t rc = (uint32_t)c->rounds;
    for (size_t r = c->rounds; r > 0; r -= 8) {
        const uint32_t *rk = c->round_keys + ((r - 8) % c->num_round_keys);

        blk[3] = (ror32(blk[3], 1) - (rol32(blk[0], 8) ^ rk[7])) ^ --rc;
        blk[2] = (ror32(blk[2], 8) - (rol32(blk[3], 1) ^ rk[6])) ^ --rc;
        blk[1] = (ror32(blk[1], 1) - (rol32(blk[2], 8) ^ rk[5])) ^ --rc;
        blk[0] = (ror32(blk[0], 8) - (rol32(blk[1], 1) ^ rk[4])) ^ --rc;

        blk[3] = (ror32(blk[3], 1) - (rol32(blk[0], 8) ^ rk[3])) ^ --rc;
        blk[2] = (ror32(blk[2], 8) - (rol32(blk[3], 1) ^ rk[2])) ^ --rc;
        blk[1] = (ror32(blk[1], 1) - (rol32(blk[2], 8) ^ rk[1])) ^ --rc;
        blk[0] = (ror32(blk[0], 8) - (rol32(blk[1], 1) ^ rk[0])) ^ --rc;
    }

    memcpy(out, blk, sizeof(blk));
}

void cham_clear(void *ctx)
{
    secure_zero(ctx, sizeof(cham_ctx));
}

const block_cipher cham64_128_block_cipher = {
    .block_size = 8,
    .key_size = 16,
    .expand_key = cham64_128_expand_key,
    .encrypt = cham64_encrypt,
    .decrypt = cham64_decrypt,
    .clear = cham_clear,
};
const block_cipher cham128_128_block_cipher = {
    .block_size = 16,
    .key_size = 16,
    .expand_key = cham128_128_expand_key,
    .encrypt = cham128_encrypt,
    .decrypt = cham128_decrypt,
    .clear = cham_clear,
};
const block_cipher cham128_256_block_cipher = {
    .block_size = 16,
    .key_size = 32,
    .expand_key = cham128_256_expand_key,
    .encrypt = cham128_encrypt,
    .decrypt = cham128_decrypt,
    .clear = cham_clear,
};
