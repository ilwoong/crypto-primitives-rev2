#include "crypto-primitives/lsh256.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DIGEST_SIZE 32

typedef struct {
    const char *name;
    size_t message_len;
    uint8_t message[8];
    uint8_t digest[DIGEST_SIZE];
} test_vector;

// Crypto++ TestVectors/lsh256.txt (LSH-256-256)
static const test_vector TEST_VECTORS[] = {
    {
        .name = "LSH-256-256 (0-byte message)",
        .message_len = 0,
        .message = {0},
        .digest = {0xf3, 0xcd, 0x41, 0x6a, 0x03, 0x81, 0x82, 0x17, 0x72, 0x6c, 0xb4, 0x7f, 0x4e, 0x4d, 0x28, 0x81,
                   0xc9, 0xc2, 0x9f, 0xd4, 0x45, 0xc1, 0x8b, 0x66, 0xfb, 0x19, 0xde, 0xa1, 0xa8, 0x10, 0x07, 0xc1},
    },
    {
        .name = "LSH-256-256 (1-byte message)",
        .message_len = 1,
        .message = {0xce},
        .digest = {0x86, 0x2f, 0x86, 0xdb, 0x65, 0x40, 0x94, 0x84, 0x0d, 0x86, 0xdf, 0x78, 0x81, 0x73, 0x2f, 0xd6,
                   0x9b, 0x72, 0x27, 0xee, 0x4f, 0x79, 0x43, 0x86, 0x81, 0x62, 0xfe, 0xb7, 0x33, 0xa9, 0xca, 0x5b},
    },
    {
        .name = "LSH-256-256 (2-byte message)",
        .message_len = 2,
        .message = {0x8b, 0x6c},
        .digest = {0xda, 0x96, 0xb2, 0x13, 0x14, 0xcf, 0xd1, 0x29, 0xfd, 0xba, 0xa6, 0x20, 0xdc, 0x3d, 0x0e, 0x2b,
                   0x5b, 0x3e, 0x08, 0x7e, 0x90, 0xe6, 0xc1, 0x47, 0xcc, 0x6b, 0x99, 0x50, 0xfd, 0xe4, 0xb4, 0x0e},
    },
    {
        .name = "LSH-256-256 (3-byte message)",
        .message_len = 3,
        .message = {0x0e, 0xc7, 0x4d},
        .digest = {0x7f, 0x23, 0x2e, 0x4c, 0xbc, 0x79, 0x6b, 0xe2, 0x27, 0xed, 0xe0, 0x18, 0xbd, 0x76, 0x92, 0x21,
                   0x33, 0x12, 0xa2, 0xc6, 0x54, 0x01, 0x3f, 0x5d, 0x06, 0x8c, 0xd0, 0x83, 0x65, 0x0a, 0xd8, 0x8a},
    },
    {
        .name = "LSH-256-256 (4-byte message)",
        .message_len = 4,
        .message = {0xa5, 0x46, 0xa6, 0x25},
        .digest = {0x48, 0xda, 0x09, 0x60, 0xd7, 0x2b, 0xab, 0x0f, 0x52, 0xf7, 0xf3, 0x3f, 0x06, 0x3f, 0x6b, 0x4f,
                   0xb9, 0xb6, 0xc7, 0x3e, 0x15, 0xd0, 0x8f, 0x86, 0x5b, 0xb6, 0x2e, 0x22, 0xfb, 0x7e, 0xaa, 0x8a},
    },
};

static void print_digest(const char *label, const uint8_t *digest)
{
    printf("    %-8s: ", label);
    for (size_t i = 0; i < DIGEST_SIZE; ++i) {
        printf("%02x", digest[i]);
    }
    printf("\n");
}

static int check_digest(const char *test_name, const char *what, const uint8_t *actual, const uint8_t *expected)
{
    if (memcmp(actual, expected, DIGEST_SIZE) == 0) {
        return 0;
    }
    printf("[FAIL] %s: %s mismatch\n", test_name, what);
    print_digest("expected", expected);
    print_digest("actual", actual);
    return 1;
}

static int run_test_vector(const message_digest *md, const test_vector *tv)
{
    lsh256_ctx ctx;
    uint8_t out[DIGEST_SIZE];
    int failures = 0;

    md->init(&ctx);
    md->update(&ctx, tv->message, tv->message_len);
    md->final(&ctx, out);
    failures += check_digest(tv->name, "single update", out, tv->digest);

    md->init(&ctx);
    for (size_t i = 0; i < tv->message_len; ++i) {
        md->update(&ctx, tv->message + i, 1);
    }
    md->final(&ctx, out);
    failures += check_digest(tv->name, "byte-wise update", out, tv->digest);

    if (failures == 0) {
        printf("[PASS] %s\n", tv->name);
    }
    return failures;
}

int main(void)
{
    int failures = 0;

    if (lsh256_message_digest.digest_size != DIGEST_SIZE || lsh256_message_digest.block_size != 128) {
        printf("[FAIL] digest_size/block_size mismatch\n");
        failures++;
    }

    for (size_t i = 0; i < sizeof(TEST_VECTORS) / sizeof(TEST_VECTORS[0]); ++i) {
        failures += run_test_vector(&lsh256_message_digest, &TEST_VECTORS[i]);
    }

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
