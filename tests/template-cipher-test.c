#include "crypto-primitives/template-cipher.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 16

typedef struct {
    const char *name;
    uint8_t key[BLOCK_SIZE];
    uint8_t plaintext[BLOCK_SIZE];
    uint8_t ciphertext[BLOCK_SIZE];
} test_vector;

static const test_vector TEST_VECTORS[] = {
    {
        .name = "zero key",
        .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
        .ciphertext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
    },
    {
        .name = "complementary nibbles",
        .key = {0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00},
        .plaintext = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
        .ciphertext = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f},
    },
    {
        .name = "all ones key",
        .key = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
        .ciphertext = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00},
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
    template_cipher_ctx ctx;
    uint8_t buf[BLOCK_SIZE];
    int failures = 0;

    cipher->expand_key(&ctx, tv->key);

    cipher->encrypt(&ctx, buf, tv->plaintext);
    failures += check_block(tv->name, "encrypt", buf, tv->ciphertext);

    cipher->decrypt(&ctx, buf, tv->ciphertext);
    failures += check_block(tv->name, "decrypt", buf, tv->plaintext);

    /* in-place (out == in) */
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
    const size_t num_vectors = sizeof(TEST_VECTORS) / sizeof(TEST_VECTORS[0]);
    int failures = 0;

    for (size_t i = 0; i < num_vectors; ++i) {
        failures += run_test_vector(&template_block_cipher, &TEST_VECTORS[i]);
    }

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
