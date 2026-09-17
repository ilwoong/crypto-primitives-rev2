#include "crypto-primitives/hight.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 8

typedef struct {
    const char *name;
    uint8_t key[16];
    uint8_t plaintext[BLOCK_SIZE];
    uint8_t ciphertext[BLOCK_SIZE];
} test_vector;

// KISA HIGHT vectors. The spec writes bytes MSB-first (K15..K0, P7..P0, C7..C0);
// these arrays store them least-significant byte first, matching the original implementation.
static const test_vector TEST_VECTORS[] = {
    {
        .name = "KISA-1",
        .key = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00},
        .plaintext = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .ciphertext = {0xf2, 0x03, 0x4f, 0xd9, 0xae, 0x18, 0xf4, 0x00},
    },
    {
        .name = "KISA-2",
        .key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
        .plaintext = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00},
        .ciphertext = {0xd8, 0xe6, 0x43, 0xe5, 0x72, 0x9f, 0xce, 0x23},
    },
    {
        .name = "KISA-3",
        .key = {0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00},
        .plaintext = {0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01},
        .ciphertext = {0x66, 0xf4, 0x23, 0x8d, 0xa2, 0xb2, 0x6f, 0x7a},
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
    hight_ctx ctx;
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
        failures += run_test_vector(&hight_block_cipher, &TEST_VECTORS[i]);
    }

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
