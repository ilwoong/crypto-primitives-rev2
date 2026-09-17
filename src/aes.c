#include "crypto-primitives/aes.h"

#include <string.h>

static const uint32_t RC[] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36,
};

static uint8_t gf256_mul(uint8_t lhs, uint8_t rhs)
{
    uint8_t result = 0;
    uint8_t carry = 0;

    for (int i = 0; i < 8; ++i) {
        if (rhs & 1) {
            result ^= lhs;
        }
        carry = lhs & 0x80;

        lhs <<= 1;
        if (carry) {
            lhs ^= 0x1b;
        }

        rhs >>= 1;
    }

    return result;
}

static uint8_t bitlength(uint16_t value)
{
    uint8_t len = 8;
    for (int i = 0; i < 16; ++i) {
        if ((value & 0x8000) == 0x8000) {
            break;
        }

        len -= 1;
        value <<= 1;
    }

    return len;
}

static uint8_t gf256_inv(uint8_t value)
{
    uint16_t u1 = 0, u3 = 0x11b, v1 = 1, v3 = value;

    while (v3 != 0) {
        uint16_t t1 = u1, t3 = u3;
        int8_t q = bitlength(u3) - bitlength(v3);

        if (q >= 0) {
            t1 ^= v1 << q;
            t3 ^= v3 << q;
        }

        u1 = v1;
        u3 = v3;
        v1 = t1;
        v3 = t3;
    }

    if (u1 >= 0x100) {
        u1 ^= 0x11b;
    }

    return u1;
}

static const uint8_t SBOX_COEF[] = {
    0xf1, 0xe3, 0xc7, 0x8f, 0x1f, 0x3e, 0x7c, 0xf8,
};

static const uint8_t SINV_COEF[] = {
    0xa4, 0x49, 0x92, 0x25, 0x4a, 0x94, 0x29, 0x52,
};

static uint8_t affine_sbox(uint8_t input)
{
    uint8_t result = 0;
    input = gf256_inv(input);
    for (int i = 0; i < 8; ++i) {
        uint8_t row = input & SBOX_COEF[i];
        uint8_t bit = 0;
        for (int j = 0; j < 8; ++j) {
            bit ^= (row & 0x1);
            row >>= 1;
        }

        result ^= (bit << i);
    }

    return result ^ 0x63;
}

static uint8_t affine_sinv(uint8_t input)
{
    uint8_t result = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t row = input & SINV_COEF[i];
        uint8_t bit = 0;
        for (int j = 0; j < 8; ++j) {
            bit ^= (row & 0x1);
            row >>= 1;
        }

        result ^= (bit << i);
    }

    return gf256_inv(result ^ 0x05);
}

static inline uint32_t rot32r8(uint32_t value)
{
    return (value >> 8) ^ (value << 24);
}

static uint32_t sub_word(uint32_t value)
{
    uint8_t *ptr = (uint8_t *)&value;

    ptr[0] = affine_sbox(ptr[0]);
    ptr[1] = affine_sbox(ptr[1]);
    ptr[2] = affine_sbox(ptr[2]);
    ptr[3] = affine_sbox(ptr[3]);

    return value;
}

static void add_round_keys(uint8_t *block, const uint8_t *rks)
{
    for (int i = 0; i < 16; ++i) {
        block[i] ^= rks[i];
    }
}

static void swap(uint8_t *block, size_t pos1, size_t pos2)
{
    uint8_t tmp = block[pos1];
    block[pos1] = block[pos2];
    block[pos2] = tmp;
}

static void sub_bytes(uint8_t *block)
{
    for (int i = 0; i < 16; ++i) {
        block[i] = affine_sbox(block[i]);
    }
}

static void shift_rows(uint8_t *block)
{
    uint8_t tmp = block[1];
    block[1] = block[5];
    block[5] = block[9];
    block[9] = block[13];
    block[13] = tmp;

    swap(block, 2, 10);
    swap(block, 6, 14);

    tmp = block[15];
    block[15] = block[11];
    block[11] = block[7];
    block[7] = block[3];
    block[3] = tmp;
}

static void mix_columns(uint8_t *in)
{
    uint8_t block[16] = {0};

    for (int i = 0; i < 16; i += 4) {
        block[i] = gf256_mul(in[i], 2) ^ gf256_mul(in[i + 1], 3) ^ gf256_mul(in[i + 2], 1) ^ gf256_mul(in[i + 3], 1);
        block[i + 1] =
            gf256_mul(in[i], 1) ^ gf256_mul(in[i + 1], 2) ^ gf256_mul(in[i + 2], 3) ^ gf256_mul(in[i + 3], 1);
        block[i + 2] =
            gf256_mul(in[i], 1) ^ gf256_mul(in[i + 1], 1) ^ gf256_mul(in[i + 2], 2) ^ gf256_mul(in[i + 3], 3);
        block[i + 3] =
            gf256_mul(in[i], 3) ^ gf256_mul(in[i + 1], 1) ^ gf256_mul(in[i + 2], 1) ^ gf256_mul(in[i + 3], 2);
    }

    memcpy(in, block, 16);
}

static inline void encrypt_round(uint8_t *block, const uint8_t *rk)
{
    sub_bytes(block);
    shift_rows(block);
    mix_columns(block);
    add_round_keys(block, rk);
}

static inline void encrypt_last_round(uint8_t *block, const uint8_t *rk)
{
    sub_bytes(block);
    shift_rows(block);
    add_round_keys(block, rk);
}

static void inv_sub_bytes(uint8_t *block)
{
    for (int i = 0; i < 16; ++i) {
        block[i] = affine_sinv(block[i]);
    }
}

static void inv_shift_rows(uint8_t *block)
{
    uint8_t tmp = block[13];
    block[13] = block[9];
    block[9] = block[5];
    block[5] = block[1];
    block[1] = tmp;

    swap(block, 2, 10);
    swap(block, 6, 14);

    tmp = block[3];
    block[3] = block[7];
    block[7] = block[11];
    block[11] = block[15];
    block[15] = tmp;
}

static void inv_mix_columns(uint8_t *in)
{
    uint8_t block[16] = {0};

    for (int i = 0; i < 16; i += 4) {
        block[i] =
            gf256_mul(in[i], 0xe) ^ gf256_mul(in[i + 1], 0xb) ^ gf256_mul(in[i + 2], 0xd) ^ gf256_mul(in[i + 3], 0x9);
        block[i + 1] =
            gf256_mul(in[i], 0x9) ^ gf256_mul(in[i + 1], 0xe) ^ gf256_mul(in[i + 2], 0xb) ^ gf256_mul(in[i + 3], 0xd);
        block[i + 2] =
            gf256_mul(in[i], 0xd) ^ gf256_mul(in[i + 1], 0x9) ^ gf256_mul(in[i + 2], 0xe) ^ gf256_mul(in[i + 3], 0xb);
        block[i + 3] =
            gf256_mul(in[i], 0xb) ^ gf256_mul(in[i + 1], 0xd) ^ gf256_mul(in[i + 2], 0x9) ^ gf256_mul(in[i + 3], 0xe);
    }

    memcpy(in, block, 16);
}

static inline void decrypt_round(uint8_t *block, const uint8_t *rk)
{
    inv_shift_rows(block);
    inv_sub_bytes(block);
    add_round_keys(block, rk);
    inv_mix_columns(block);
}

static inline void decrypt_last_round(uint8_t *block, const uint8_t *rk)
{
    inv_sub_bytes(block);
    inv_shift_rows(block);
    add_round_keys(block, rk);
}

const block_cipher aes128_block_cipher = {
    .block_size = 16,
    .key_size = 16,
    .expand_key = aes128_expand_key,
    .encrypt = aes_encrypt,
    .decrypt = aes_decrypt,
};

const block_cipher aes192_block_cipher = {
    .block_size = 16,
    .key_size = 24,
    .expand_key = aes192_expand_key,
    .encrypt = aes_encrypt,
    .decrypt = aes_decrypt,
};

const block_cipher aes256_block_cipher = {
    .block_size = 16,
    .key_size = 32,
    .expand_key = aes256_expand_key,
    .encrypt = aes_encrypt,
    .decrypt = aes_decrypt,
};

void aes128_expand_key(void *ctx, const uint8_t *master_key)
{
    aes_ctx *c = (aes_ctx *)ctx;
    c->rounds = 10;

    uint32_t *rk = (uint32_t *)c->round_keys;
    memcpy(rk, master_key, 16);

    for (int i = 0; i < 10; ++i) {
        rk[4] = rk[0] ^ sub_word(rot32r8(rk[3])) ^ RC[i];
        rk[5] = rk[1] ^ rk[4];
        rk[6] = rk[2] ^ rk[5];
        rk[7] = rk[3] ^ rk[6];
        rk += 4;
    }
}

void aes192_expand_key(void *ctx, const uint8_t *master_key)
{
    aes_ctx *c = (aes_ctx *)ctx;
    c->rounds = 12;

    uint32_t *rk = (uint32_t *)c->round_keys;
    memcpy(rk, master_key, 24);

    for (int i = 0; i < 7; ++i) {
        rk[6] = rk[0] ^ sub_word(rot32r8(rk[5])) ^ RC[i];
        rk[7] = rk[1] ^ rk[6];
        rk[8] = rk[2] ^ rk[7];
        rk[9] = rk[3] ^ rk[8];
        rk[10] = rk[4] ^ rk[9];
        rk[11] = rk[5] ^ rk[10];

        rk += 6;
    }

    rk[6] = rk[0] ^ sub_word(rot32r8(rk[5])) ^ RC[7];
    rk[7] = rk[1] ^ rk[6];
    rk[8] = rk[2] ^ rk[7];
    rk[9] = rk[3] ^ rk[8];
}

void aes256_expand_key(void *ctx, const uint8_t *master_key)
{
    aes_ctx *c = (aes_ctx *)ctx;
    c->rounds = 14;

    uint32_t *rk = (uint32_t *)c->round_keys;
    memcpy(rk, master_key, 32);

    for (int i = 0; i < 7; ++i) {
        rk[8] = rk[0] ^ sub_word(rot32r8(rk[7])) ^ RC[i];
        rk[9] = rk[1] ^ rk[8];
        rk[10] = rk[2] ^ rk[9];
        rk[11] = rk[3] ^ rk[10];

        if (i == 6) {
            break;
        }

        rk[12] = rk[4] ^ sub_word(rk[11]);
        rk[13] = rk[5] ^ rk[12];
        rk[14] = rk[6] ^ rk[13];
        rk[15] = rk[7] ^ rk[14];

        rk += 8;
    }
}

void aes_encrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    aes_ctx *c = (aes_ctx *)ctx;
    const uint8_t *rks = c->round_keys;
    uint8_t block[16] = {0};
    memcpy(block, in, 16);

    add_round_keys(block, rks);
    rks += 16;

    for (size_t i = 0; i < c->rounds - 1; ++i, rks += 16) {
        encrypt_round(block, rks);
    }

    encrypt_last_round(block, rks);

    memcpy(out, block, 16);
}

void aes_decrypt(void *ctx, uint8_t *out, const uint8_t *in)
{
    aes_ctx *c = (aes_ctx *)ctx;
    const uint8_t *rks = c->round_keys;
    uint8_t block[16] = {0};
    memcpy(block, in, 16);

    rks += 16 * c->rounds;

    add_round_keys(block, rks);
    rks -= 16;

    for (size_t i = 0; i < c->rounds - 1; ++i, rks -= 16) {
        decrypt_round(block, rks);
    }

    decrypt_last_round(block, rks);

    memcpy(out, block, 16);
}
