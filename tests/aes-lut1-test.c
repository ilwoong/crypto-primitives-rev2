#include "crypto-primitives/aes-lut1.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 16

typedef struct {
    const char *name;
    uint8_t key[32];
    uint8_t plaintext[BLOCK_SIZE];
    uint8_t ciphertext[BLOCK_SIZE];
} test_vector;

static const test_vector TV_AES128 = {
    .name = "AES-128",
    .key = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
    .plaintext = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34},
    .ciphertext = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32},
};

static const test_vector TV_AES192 = {
    .name = "AES-192",
    .key = {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b,
            0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b},
    .plaintext = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
    .ciphertext = {0xbd, 0x33, 0x4f, 0x1d, 0x6e, 0x45, 0xf2, 0x5f, 0xf7, 0x12, 0xa2, 0x14, 0x57, 0x1f, 0xa5, 0xcc},
};

static const test_vector TV_AES256 = {
    .name = "AES-256",
    .key = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
            0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4},
    .plaintext = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
    .ciphertext = {0xf3, 0xee, 0xd1, 0xbd, 0xb5, 0xd2, 0xa0, 0x3c, 0x06, 0x4b, 0x5a, 0x7e, 0x3d, 0xb1, 0x81, 0xf8},
};

static void print_block(const char *label, const uint8_t *block)
{
    printf("    %-8s: ", label);
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        printf("%02x", block[i]);
    }
    printf("\n");
}

static int check_block(const char *test_name, const char *what, const uint8_t *actual, const uint8_t *expected)
{
    if (memcmp(actual, expected, BLOCK_SIZE) == 0) {
        return 0;
    }
    printf("[FAIL] %s: %s mismatch\n", test_name, what);
    print_block("expected", expected);
    print_block("actual", actual);
    return 1;
}

static int run_test_vector(const block_cipher *cipher, const test_vector *tv)
{
    aes_lut1_ctx ctx;
    uint8_t buf[BLOCK_SIZE];
    int failures = 0;

    cipher->expand_key(&ctx, tv->key);

    cipher->encrypt(&ctx, buf, tv->plaintext);
    failures += check_block(tv->name, "encrypt", buf, tv->ciphertext);

    cipher->decrypt(&ctx, buf, tv->ciphertext);
    failures += check_block(tv->name, "decrypt", buf, tv->plaintext);

    memcpy(buf, tv->plaintext, BLOCK_SIZE);
    cipher->encrypt(&ctx, buf, buf);
    failures += check_block(tv->name, "in-place encrypt", buf, tv->ciphertext);

    cipher->decrypt(&ctx, buf, buf);
    failures += check_block(tv->name, "in-place decrypt", buf, tv->plaintext);

    if (failures == 0) {
        printf("[PASS] %s\n", tv->name);
    }
    return failures;
}

int main(void)
{
    int failures = 0;

    failures += run_test_vector(&aes128_lut1_block_cipher, &TV_AES128);
    failures += run_test_vector(&aes192_lut1_block_cipher, &TV_AES192);
    failures += run_test_vector(&aes256_lut1_block_cipher, &TV_AES256);

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
