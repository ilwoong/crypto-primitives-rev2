#include "crypto-primitives/seed.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 16

typedef struct {
    const char *name;
    uint8_t key[16];
    uint8_t plaintext[BLOCK_SIZE];
    uint8_t ciphertext[BLOCK_SIZE];
} test_vector;

// RFC 4269 Appendix B
static const test_vector TEST_VECTORS[] = {
    {
        .name = "RFC4269 B.1",
        .key = {0},
        .plaintext = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
        .ciphertext = {0x5e, 0xba, 0xc6, 0xe0, 0x05, 0x4e, 0x16, 0x68, 0x19, 0xaf, 0xf1, 0xcc, 0x6d, 0x34, 0x6c, 0xdb},
    },
    {
        .name = "RFC4269 B.2",
        .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
        .plaintext = {0},
        .ciphertext = {0xc1, 0x1f, 0x22, 0xf2, 0x01, 0x40, 0x50, 0x50, 0x84, 0x48, 0x35, 0x97, 0xe4, 0x37, 0x0f, 0x43},
    },
    {
        .name = "RFC4269 B.3",
        .key = {0x47, 0x06, 0x48, 0x08, 0x51, 0xe6, 0x1b, 0xe8, 0x5d, 0x74, 0xbf, 0xb3, 0xfd, 0x95, 0x61, 0x85},
        .plaintext = {0x83, 0xa2, 0xf8, 0xa2, 0x88, 0x64, 0x1f, 0xb9, 0xa4, 0xe9, 0xa5, 0xcc, 0x2f, 0x13, 0x1c, 0x7d},
        .ciphertext = {0xee, 0x54, 0xd1, 0x3e, 0xbc, 0xae, 0x70, 0x6d, 0x22, 0x6b, 0xc3, 0x14, 0x2c, 0xd4, 0x0d, 0x4a},
    },
    {
        .name = "RFC4269 B.4",
        .key = {0x28, 0xdb, 0xc3, 0xbc, 0x49, 0xff, 0xd8, 0x7d, 0xcf, 0xa5, 0x09, 0xb1, 0x1d, 0x42, 0x2b, 0xe7},
        .plaintext = {0xb4, 0x1e, 0x6b, 0xe2, 0xeb, 0xa8, 0x4a, 0x14, 0x8e, 0x2e, 0xed, 0x84, 0x59, 0x3c, 0x5e, 0xc7},
        .ciphertext = {0x9b, 0x9b, 0x7b, 0xfc, 0xd1, 0x81, 0x3c, 0xb9, 0x5d, 0x0b, 0x36, 0x18, 0xf4, 0x0f, 0x51, 0x22},
    },
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
    seed_ctx ctx;
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
    for (size_t i = 0; i < sizeof(TEST_VECTORS) / sizeof(TEST_VECTORS[0]); ++i) {
        failures += run_test_vector(&seed_block_cipher, &TEST_VECTORS[i]);
    }

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
