#include "crypto-primitives/hight.h"

#include <stddef.h>

#define HIGHT_ROUNDS 32

static inline uint8_t rol8(uint8_t value, unsigned rot)
{
    return (uint8_t)((value << rot) | (value >> (8 - rot)));
}

static inline uint8_t f0(uint8_t x)
{
    return rol8(x, 1) ^ rol8(x, 2) ^ rol8(x, 7);
}

static inline uint8_t f1(uint8_t x)
{
    return rol8(x, 3) ^ rol8(x, 4) ^ rol8(x, 6);
}

static void generate_constants(uint8_t *delta)
{
    uint8_t s[134] = {0, 1, 0, 1, 1, 0, 1};

    delta[0] = 0x5a;
    for (size_t i = 1; i < 128; ++i) {
        s[i + 6] = s[i + 2] ^ s[i - 1];

        uint8_t d = s[i + 6];
        for (size_t j = 1; j < 7; ++j) {
            d = (uint8_t)((d << 1) ^ s[i + 6 - j]);
        }
        delta[i] = d;
    }
}

void hight_expand_key(void *ctx, const uint8_t *master_key)
{
    hight_ctx *c = (hight_ctx *)ctx;

    for (size_t i = 0; i < 4; ++i) {
        c->whitening_keys[i] = master_key[i + 12];
        c->whitening_keys[i + 4] = master_key[i];
    }

    uint8_t delta[128];
    generate_constants(delta);

    for (size_t i = 0; i < 8; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            size_t index = (j + 8 - i) & 7;
            c->round_keys[16 * i + j] = (uint8_t)(master_key[index] + delta[16 * i + j]);
            c->round_keys[16 * i + j + 8] = (uint8_t)(master_key[index + 8] + delta[16 * i + j + 8]);
        }
    }
}

void hight_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const hight_ctx *c = (const hight_ctx *)ctx;
    const uint8_t *wk = c->whitening_keys;
    const uint8_t *rk = c->round_keys;
    uint8_t b[8];

    b[0] = (uint8_t)(in[0] + wk[0]);
    b[1] = in[1];
    b[2] = in[2] ^ wk[1];
    b[3] = in[3];
    b[4] = (uint8_t)(in[4] + wk[2]);
    b[5] = in[5];
    b[6] = in[6] ^ wk[3];
    b[7] = in[7];

    for (size_t r = 0; r < HIGHT_ROUNDS; ++r, rk += 4) {
        uint8_t t6 = b[6];
        uint8_t t7 = b[7];

        b[7] = b[6];
        b[6] = (uint8_t)(b[5] + (f1(b[4]) ^ rk[2]));
        b[5] = b[4];
        b[4] = b[3] ^ (uint8_t)(f0(b[2]) + rk[1]);
        b[3] = b[2];
        b[2] = (uint8_t)(b[1] + (f1(b[0]) ^ rk[0]));
        b[1] = b[0];
        b[0] = t7 ^ (uint8_t)(f0(t6) + rk[3]);
    }

    out[0] = (uint8_t)(b[1] + wk[4]);
    out[1] = b[2];
    out[2] = b[3] ^ wk[5];
    out[3] = b[4];
    out[4] = (uint8_t)(b[5] + wk[6]);
    out[5] = b[6];
    out[6] = b[7] ^ wk[7];
    out[7] = b[0];
}

void hight_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    const hight_ctx *c = (const hight_ctx *)ctx;
    const uint8_t *wk = c->whitening_keys;
    const uint8_t *rk = c->round_keys + 4 * (HIGHT_ROUNDS - 1);
    uint8_t b[8];

    b[7] = in[6] ^ wk[7];
    b[6] = in[5];
    b[5] = (uint8_t)(in[4] - wk[6]);
    b[4] = in[3];
    b[3] = in[2] ^ wk[5];
    b[2] = in[1];
    b[1] = (uint8_t)(in[0] - wk[4]);
    b[0] = in[7];

    for (size_t r = 0; r < HIGHT_ROUNDS; ++r, rk -= 4) {
        uint8_t t0 = b[0];

        b[0] = b[1];
        b[1] = (uint8_t)(b[2] - (f1(b[0]) ^ rk[0]));
        b[2] = b[3];
        b[3] = b[4] ^ (uint8_t)(f0(b[2]) + rk[1]);
        b[4] = b[5];
        b[5] = (uint8_t)(b[6] - (f1(b[4]) ^ rk[2]));
        b[6] = b[7];
        b[7] = t0 ^ (uint8_t)(f0(b[6]) + rk[3]);
    }

    out[0] = (uint8_t)(b[0] - wk[0]);
    out[1] = b[1];
    out[2] = b[2] ^ wk[1];
    out[3] = b[3];
    out[4] = (uint8_t)(b[4] - wk[2]);
    out[5] = b[5];
    out[6] = b[6] ^ wk[3];
    out[7] = b[7];
}

void hight_clear(void *ctx)
{
    secure_zero(ctx, sizeof(hight_ctx));
}

const block_cipher hight_block_cipher = {
    .block_size = 8,
    .key_size = 16,
    .expand_key = hight_expand_key,
    .encrypt = hight_encrypt,
    .decrypt = hight_decrypt,
    .clear = hight_clear,
};
