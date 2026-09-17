#include "crypto-primitives/lea.h"

#include <string.h>

#define LEA128_ROUNDS 24
#define LEA192_ROUNDS 28
#define LEA256_ROUNDS 32

static const uint32_t DELTA[8] = {
    0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957,
};

// The key schedule passes rotation amounts up to 36 (spec: ROL by (i + k) mod 32).
static inline uint32_t rol32(uint32_t value, unsigned rot)
{
    rot &= 31;
    return (value << rot) | (value >> ((32 - rot) & 31));
}

static inline uint32_t ror32(uint32_t value, unsigned rot)
{
    rot &= 31;
    return (value >> rot) | (value << ((32 - rot) & 31));
}

void lea128_expand_key(void *ctx, const uint8_t *master_key)
{
    lea_ctx *c = (lea_ctx *)ctx;
    c->rounds = LEA128_ROUNDS;

    uint32_t t[4];
    memcpy(t, master_key, 16);

    uint32_t *rk = c->round_keys;
    for (size_t i = 0; i < LEA128_ROUNDS; ++i) {
        uint32_t delta = DELTA[i & 3];
        t[0] = rol32(t[0] + rol32(delta, i), 1);
        t[1] = rol32(t[1] + rol32(delta, i + 1), 3);
        t[2] = rol32(t[2] + rol32(delta, i + 2), 6);
        t[3] = rol32(t[3] + rol32(delta, i + 3), 11);

        rk[0] = t[0];
        rk[1] = t[1];
        rk[2] = t[2];
        rk[3] = t[1];
        rk[4] = t[3];
        rk[5] = t[1];
        rk += 6;
    }
}

void lea192_expand_key(void *ctx, const uint8_t *master_key)
{
    lea_ctx *c = (lea_ctx *)ctx;
    c->rounds = LEA192_ROUNDS;

    uint32_t t[6];
    memcpy(t, master_key, 24);

    uint32_t *rk = c->round_keys;
    for (size_t i = 0; i < LEA192_ROUNDS; ++i) {
        uint32_t delta = DELTA[i % 6];
        t[0] = rol32(t[0] + rol32(delta, i), 1);
        t[1] = rol32(t[1] + rol32(delta, i + 1), 3);
        t[2] = rol32(t[2] + rol32(delta, i + 2), 6);
        t[3] = rol32(t[3] + rol32(delta, i + 3), 11);
        t[4] = rol32(t[4] + rol32(delta, i + 4), 13);
        t[5] = rol32(t[5] + rol32(delta, i + 5), 17);

        rk[0] = t[0];
        rk[1] = t[1];
        rk[2] = t[2];
        rk[3] = t[3];
        rk[4] = t[4];
        rk[5] = t[5];
        rk += 6;
    }
}

void lea256_expand_key(void *ctx, const uint8_t *master_key)
{
    lea_ctx *c = (lea_ctx *)ctx;
    c->rounds = LEA256_ROUNDS;

    uint32_t t[8];
    memcpy(t, master_key, 32);

    uint32_t *rk = c->round_keys;
    for (size_t i = 0; i < LEA256_ROUNDS; ++i) {
        uint32_t delta = DELTA[i & 7];
        t[(6 * i) & 7] = rol32(t[(6 * i) & 7] + rol32(delta, i), 1);
        t[(6 * i + 1) & 7] = rol32(t[(6 * i + 1) & 7] + rol32(delta, i + 1), 3);
        t[(6 * i + 2) & 7] = rol32(t[(6 * i + 2) & 7] + rol32(delta, i + 2), 6);
        t[(6 * i + 3) & 7] = rol32(t[(6 * i + 3) & 7] + rol32(delta, i + 3), 11);
        t[(6 * i + 4) & 7] = rol32(t[(6 * i + 4) & 7] + rol32(delta, i + 4), 13);
        t[(6 * i + 5) & 7] = rol32(t[(6 * i + 5) & 7] + rol32(delta, i + 5), 17);

        rk[0] = t[(6 * i) & 7];
        rk[1] = t[(6 * i + 1) & 7];
        rk[2] = t[(6 * i + 2) & 7];
        rk[3] = t[(6 * i + 3) & 7];
        rk[4] = t[(6 * i + 4) & 7];
        rk[5] = t[(6 * i + 5) & 7];
        rk += 6;
    }
}

void lea_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    lea_ctx *c = (lea_ctx *)ctx;
    const uint32_t *rk = c->round_keys;

    uint32_t b[4];
    memcpy(b, in, 16);

    for (size_t i = 0; i < c->rounds; ++i) {
        uint32_t tmp = b[0];
        b[0] = rol32((b[0] ^ rk[0]) + (b[1] ^ rk[1]), 9);
        b[1] = ror32((b[1] ^ rk[2]) + (b[2] ^ rk[3]), 5);
        b[2] = ror32((b[2] ^ rk[4]) + (b[3] ^ rk[5]), 3);
        b[3] = tmp;
        rk += 6;
    }

    memcpy(out, b, 16);
}

void lea_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    lea_ctx *c = (lea_ctx *)ctx;
    // Walk down from one past the end so the pointer never goes below the array.
    const uint32_t *rk = c->round_keys + 6 * c->rounds;

    uint32_t b[4];
    memcpy(b, in, 16);

    for (size_t i = 0; i < c->rounds; ++i) {
        rk -= 6;
        uint32_t a = b[3];
        uint32_t new1 = (ror32(b[0], 9) - (b[3] ^ rk[0])) ^ rk[1];
        uint32_t new2 = (rol32(b[1], 5) - (new1 ^ rk[2])) ^ rk[3];
        uint32_t new3 = (rol32(b[2], 3) - (new2 ^ rk[4])) ^ rk[5];
        b[0] = a;
        b[1] = new1;
        b[2] = new2;
        b[3] = new3;
    }

    memcpy(out, b, 16);
}

void lea_clear(void *ctx)
{
    secure_zero(ctx, sizeof(lea_ctx));
}

const block_cipher lea128_block_cipher = {
    .block_size = 16,
    .key_size = 16,
    .expand_key = lea128_expand_key,
    .encrypt = lea_encrypt,
    .decrypt = lea_decrypt,
    .clear = lea_clear,
};
const block_cipher lea192_block_cipher = {
    .block_size = 16,
    .key_size = 24,
    .expand_key = lea192_expand_key,
    .encrypt = lea_encrypt,
    .decrypt = lea_decrypt,
    .clear = lea_clear,
};
const block_cipher lea256_block_cipher = {
    .block_size = 16,
    .key_size = 32,
    .expand_key = lea256_expand_key,
    .encrypt = lea_encrypt,
    .decrypt = lea_decrypt,
    .clear = lea_clear,
};
