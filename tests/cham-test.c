#include "crypto-primitives/cham.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_BLOCK_SIZE 16

typedef struct {
    const char *name;
    uint8_t key[32];
    uint8_t plaintext[MAX_BLOCK_SIZE];
    uint8_t ciphertext[MAX_BLOCK_SIZE];
} test_vector;

static const test_vector TV_CHAM64_128 = {
    .name = "CHAM-64/128",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77},
    .ciphertext = {0x79, 0x65, 0x04, 0x12, 0x3f, 0x12, 0xa9, 0xe5},
};

static const test_vector TV_CHAM128_128 = {
    .name = "CHAM-128/128",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
    .ciphertext = {0xee, 0x19, 0x54, 0xd0, 0x4c, 0x8f, 0x11, 0x9f, 0x69, 0x64, 0xe3, 0x99, 0xc1, 0x5e, 0x88, 0x1c},
};

static const test_vector TV_CHAM128_256 = {
    .name = "CHAM-128/256",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
    .ciphertext = {0xdc, 0x77, 0x73, 0x02, 0x51, 0x56, 0x0b, 0x12, 0x95, 0x9b, 0x83, 0x8f, 0x75, 0xc0, 0x5e, 0x5e},
};

static void print_block(const char *label, const uint8_t *block, size_t size)
{
    printf("    %-8s: ", label);
    for (size_t i = 0; i < size; ++i) {
        printf("%02x", block[i]);
    }
    printf("\n");
}

static int check_block(const char *test_name, const char *what, const uint8_t *actual, const uint8_t *expected,
                       size_t size)
{
    if (memcmp(actual, expected, size) == 0) {
        return 0;
    }
    printf("[FAIL] %s: %s mismatch\n", test_name, what);
    print_block("expected", expected, size);
    print_block("actual", actual, size);
    return 1;
}

static int check_cleared(const char *test_name, const void *ctx, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)ctx;
    for (size_t i = 0; i < size; ++i) {
        if (bytes[i] != 0) {
            printf("[FAIL] %s: clear left nonzero bytes\n", test_name);
            return 1;
        }
    }
    return 0;
}

static int run_test_vector(const block_cipher *cipher, const test_vector *tv)
{
    cham_ctx ctx;
    uint8_t buf[MAX_BLOCK_SIZE];
    int failures = 0;

    if (cipher->block_size > MAX_BLOCK_SIZE) {
        printf("[FAIL] %s: block_size %zu exceeds buffer %d\n", tv->name, cipher->block_size, MAX_BLOCK_SIZE);
        return 1;
    }

    if (!cipher->clear) {
        printf("[FAIL] %s: clear is NULL\n", tv->name);
        return 1;
    }

    cipher->expand_key(&ctx, tv->key);

    cipher->encrypt(&ctx, buf, tv->plaintext);
    failures += check_block(tv->name, "encrypt", buf, tv->ciphertext, cipher->block_size);

    cipher->decrypt(&ctx, buf, tv->ciphertext);
    failures += check_block(tv->name, "decrypt", buf, tv->plaintext, cipher->block_size);

    memcpy(buf, tv->plaintext, cipher->block_size);
    cipher->encrypt(&ctx, buf, buf);
    failures += check_block(tv->name, "in-place encrypt", buf, tv->ciphertext, cipher->block_size);

    cipher->decrypt(&ctx, buf, buf);
    failures += check_block(tv->name, "in-place decrypt", buf, tv->plaintext, cipher->block_size);

    cipher->clear(&ctx);
    failures += check_cleared(tv->name, &ctx, sizeof(ctx));

    if (failures == 0) {
        printf("[PASS] %s\n", tv->name);
    }
    return failures;
}

int main(void)
{
    int failures = 0;

    failures += run_test_vector(&cham64_128_block_cipher, &TV_CHAM64_128);
    failures += run_test_vector(&cham128_128_block_cipher, &TV_CHAM128_128);
    failures += run_test_vector(&cham128_256_block_cipher, &TV_CHAM128_256);

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
