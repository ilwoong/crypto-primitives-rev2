#include "crypto-primitives/lsh256.h"

#include <string.h>

#define NUM_STEPS 26
#define BLOCK_SIZE 128
#define DIGEST_SIZE 32

#define ALPHA_EVEN 29
#define ALPHA_ODD 5
#define BETA_EVEN 1
#define BETA_ODD 17

static const uint32_t IV[16] = {
    0x46a10f1f, 0xfddce486, 0xb41443a8, 0x198e6b9d, 0x3304388d, 0xb0f5a3c7, 0xb36061c4, 0x7adbd553,
    0x105d5378, 0x2f74de54, 0x5c2f2d95, 0xf2553fbe, 0x8051357a, 0x138668c8, 0x47aa4484, 0xe01afb41,
};

static const uint32_t STEP_CONSTANT[208] = {
    0x917caf90, 0x6c1b10a2, 0x6f352943, 0xcf778243, 0x2ceb7472, 0x29e96ff2, 0x8a9ba428, 0x2eeb2642, 0x0e2c4021,
    0x872bb30e, 0xa45e6cb2, 0x46f9c612, 0x185fe69e, 0x1359621b, 0x263fccb2, 0x1a116870, 0x3a6c612f, 0xb2dec195,
    0x02cb1f56, 0x40bfd858, 0x784684b6, 0x6cbb7d2e, 0x660c7ed8, 0x2b79d88a, 0xa6cd9069, 0x91a05747, 0xcdea7558,
    0x00983098, 0xbecb3b2e, 0x2838ab9a, 0x728b573e, 0xa55262b5, 0x745dfa0f, 0x31f79ed8, 0xb85fce25, 0x98c8c898,
    0x8a0669ec, 0x60e445c2, 0xfde295b0, 0xf7b5185a, 0xd2580983, 0x29967709, 0x182df3dd, 0x61916130, 0x90705676,
    0x452a0822, 0xe07846ad, 0xaccd7351, 0x2a618d55, 0xc00d8032, 0x4621d0f5, 0xf2f29191, 0x00c6cd06, 0x6f322a67,
    0x58bef48d, 0x7a40c4fd, 0x8beee27f, 0xcd8db2f2, 0x67f2c63b, 0xe5842383, 0xc793d306, 0xa15c91d6, 0x17b381e5,
    0xbb05c277, 0x7ad1620a, 0x5b40a5bf, 0x5ab901a2, 0x69a7a768, 0x5b66d9cd, 0xfdee6877, 0xcb3566fc, 0xc0c83a32,
    0x4c336c84, 0x9be6651a, 0x13baa3fc, 0x114f0fd1, 0xc240a728, 0xec56e074, 0x009c63c7, 0x89026cf2, 0x7f9ff0d0,
    0x824b7fb5, 0xce5ea00f, 0x605ee0e2, 0x02e7cfea, 0x43375560, 0x9d002ac7, 0x8b6f5f7b, 0x1f90c14f, 0xcdcb3537,
    0x2cfeafdd, 0xbf3fc342, 0xeab7b9ec, 0x7a8cb5a3, 0x9d2af264, 0xfacedb06, 0xb052106e, 0x99006d04, 0x2bae8d09,
    0xff030601, 0xa271a6d6, 0x0742591d, 0xc81d5701, 0xc9a9e200, 0x02627f1e, 0x996d719d, 0xda3b9634, 0x02090800,
    0x14187d78, 0x499b7624, 0xe57458c9, 0x738be2c9, 0x64e19d20, 0x06df0f36, 0x15d1cb0e, 0x0b110802, 0x2c95f58c,
    0xe5119a6d, 0x59cd22ae, 0xff6eac3c, 0x467ebd84, 0xe5ee453c, 0xe79cd923, 0x1c190a0d, 0xc28b81b8, 0xf6ac0852,
    0x26efd107, 0x6e1ae93b, 0xc53c41ca, 0xd4338221, 0x8475fd0a, 0x35231729, 0x4e0d3a7a, 0xa2b45b48, 0x16c0d82d,
    0x890424a9, 0x017e0c8f, 0x07b5a3f5, 0xfa73078e, 0x583a405e, 0x5b47b4c8, 0x570fa3ea, 0xd7990543, 0x8d28ce32,
    0x7f8a9b90, 0xbd5998fc, 0x6d7a9688, 0x927a9eb6, 0xa2fc7d23, 0x66b38e41, 0x709e491a, 0xb5f700bf, 0x0a262c0f,
    0x16f295b9, 0xe8111ef5, 0x0d195548, 0x9f79a0c5, 0x1a41cfa7, 0x0ee7638a, 0xacf7c074, 0x30523b19, 0x09884ecf,
    0xf93014dd, 0x266e9d55, 0x191a6664, 0x5c1176c1, 0xf64aed98, 0xa4b83520, 0x828d5449, 0x91d71dd8, 0x2944f2d6,
    0x950bf27b, 0x3380ca7d, 0x6d88381d, 0x4138868e, 0x5ced55c4, 0x0fe19dcb, 0x68f4f669, 0x6e37c8ff, 0xa0fe6e10,
    0xb44b47b0, 0xf5c0558a, 0x79bf14cf, 0x4a431a20, 0xf17f68da, 0x5deb5fd1, 0xa600c86d, 0x9f6c7eb0, 0xff92f864,
    0xb615e07f, 0x38d3e448, 0x8d5d3a6a, 0x70e843cb, 0x494b312e, 0xa6c93613, 0x0beb2f4f, 0x928b5d63, 0xcbf66035,
    0x0cb82c80, 0xea97a4f7, 0x592c0f3b, 0x947c5f77, 0x6fff49b9, 0xf71a7e5a, 0x1de8c0f5, 0xc2569600, 0xc4e4ac8c,
    0x823c9ce1,
};

static const unsigned GAMMA[8] = {0, 8, 16, 24, 24, 16, 8, 0};

// GAMMA contains 0, so the shift by (width - rot) must be masked to stay defined.
static inline uint32_t rol32(uint32_t value, unsigned rot)
{
    rot &= 31;
    return (value << rot) | (value >> ((32 - rot) & 31));
}

static inline uint32_t load_le32(const uint8_t *p)
{
    uint32_t v = 0;
    for (size_t i = 0; i < 4; ++i) {
        v |= (uint32_t)p[i] << (8 * i);
    }
    return v;
}

static inline void store_le32(uint8_t *p, uint32_t v)
{
    for (size_t i = 0; i < 4; ++i) {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

static void expand_message(lsh256_ctx *c, const uint8_t *in)
{
    uint32_t *msg = c->msg;
    for (size_t i = 0; i < 32; ++i) {
        msg[i] = load_le32(in + 4 * i);
    }

    for (size_t i = 2; i <= NUM_STEPS; ++i) {
        size_t idx = 16 * i;
        msg[idx] = msg[idx - 16] + msg[idx - 29];
        msg[idx + 1] = msg[idx - 15] + msg[idx - 30];
        msg[idx + 2] = msg[idx - 14] + msg[idx - 32];
        msg[idx + 3] = msg[idx - 13] + msg[idx - 31];
        msg[idx + 4] = msg[idx - 12] + msg[idx - 25];
        msg[idx + 5] = msg[idx - 11] + msg[idx - 28];
        msg[idx + 6] = msg[idx - 10] + msg[idx - 27];
        msg[idx + 7] = msg[idx - 9] + msg[idx - 26];
        msg[idx + 8] = msg[idx - 8] + msg[idx - 21];
        msg[idx + 9] = msg[idx - 7] + msg[idx - 22];
        msg[idx + 10] = msg[idx - 6] + msg[idx - 24];
        msg[idx + 11] = msg[idx - 5] + msg[idx - 23];
        msg[idx + 12] = msg[idx - 4] + msg[idx - 17];
        msg[idx + 13] = msg[idx - 3] + msg[idx - 20];
        msg[idx + 14] = msg[idx - 2] + msg[idx - 19];
        msg[idx + 15] = msg[idx - 1] + msg[idx - 18];
    }
}

static void permute_words(lsh256_ctx *c)
{
    static const uint8_t PERM[16] = {6, 4, 5, 7, 12, 15, 14, 13, 2, 0, 1, 3, 8, 11, 10, 9};
    for (size_t i = 0; i < 16; ++i) {
        c->cv[i] = c->tcv[PERM[i]];
    }
}

static void step(lsh256_ctx *c, size_t idx, unsigned alpha, unsigned beta)
{
    for (size_t col = 0; col < 8; ++col) {
        uint32_t vl = c->cv[col] ^ c->msg[16 * idx + col];
        uint32_t vr = c->cv[col + 8] ^ c->msg[16 * idx + col + 8];

        vl = rol32(vl + vr, alpha) ^ STEP_CONSTANT[8 * idx + col];
        vr = rol32(vl + vr, beta);

        c->tcv[col] = vl + vr;
        c->tcv[col + 8] = rol32(vr, GAMMA[col]);
    }

    permute_words(c);
}

static void compress(lsh256_ctx *c, const uint8_t *block)
{
    expand_message(c, block);

    for (size_t i = 0; i < NUM_STEPS; i += 2) {
        step(c, i, ALPHA_EVEN, BETA_EVEN);
        step(c, i + 1, ALPHA_ODD, BETA_ODD);
    }

    for (size_t i = 0; i < 16; ++i) {
        c->cv[i] ^= c->msg[16 * NUM_STEPS + i];
    }
}

void lsh256_init(void *ctx)
{
    lsh256_ctx *c = (lsh256_ctx *)ctx;
    c->block_index = 0;
    memset(c->block, 0, sizeof(c->block));
    memcpy(c->cv, IV, sizeof(c->cv));
    memset(c->tcv, 0, sizeof(c->tcv));
    memset(c->msg, 0, sizeof(c->msg));
}

void lsh256_update(void *ctx, const uint8_t *in, size_t len)
{
    lsh256_ctx *c = (lsh256_ctx *)ctx;

    if (c->block_index > 0) {
        size_t gap = BLOCK_SIZE - c->block_index;
        if (len < gap) {
            memcpy(c->block + c->block_index, in, len);
            c->block_index += len;
            return;
        }
        memcpy(c->block + c->block_index, in, gap);
        compress(c, c->block);
        c->block_index = 0;
        in += gap;
        len -= gap;
    }

    while (len >= BLOCK_SIZE) {
        compress(c, in);
        in += BLOCK_SIZE;
        len -= BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(c->block, in, len);
        c->block_index = len;
    }
}

void lsh256_final(void *ctx, uint8_t *out)
{
    lsh256_ctx *c = (lsh256_ctx *)ctx;

    c->block[c->block_index++] = 0x80;
    memset(c->block + c->block_index, 0, BLOCK_SIZE - c->block_index);
    compress(c, c->block);

    for (size_t i = 0; i < 8; ++i) {
        store_le32(out + 4 * i, c->cv[i] ^ c->cv[i + 8]);
    }

    secure_zero(c, sizeof(*c));
    lsh256_init(c);
}

const message_digest lsh256_message_digest = {DIGEST_SIZE, BLOCK_SIZE, lsh256_init, lsh256_update, lsh256_final};
