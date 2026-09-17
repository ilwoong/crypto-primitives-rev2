#include "crypto-primitives/template-cipher.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_BLOCK_SIZE 16
#define MAX_KEY_SIZE 16

typedef struct {
    const char *name;
    uint8_t key[MAX_KEY_SIZE];
    uint8_t plaintext[MAX_BLOCK_SIZE];
    uint8_t ciphertext[MAX_BLOCK_SIZE];
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

static int check_sizes(const block_cipher *cipher)
{
    int failures = 0;

    if (cipher->block_size != 16) {
        printf("[FAIL] block_size: expected 16, got %zu\n", cipher->block_size);
        failures++;
    }
    if (cipher->key_size != 16) {
        printf("[FAIL] key_size: expected 16, got %zu\n", cipher->key_size);
        failures++;
    }
    if (failures == 0) {
        printf("[PASS] size fields\n");
    }
    return failures;
}

static int run_test_vector(const block_cipher *cipher, const test_vector *tv)
{
    template_cipher_ctx ctx;
    uint8_t buf[MAX_BLOCK_SIZE];
    int failures = 0;

    if (cipher->block_size > MAX_BLOCK_SIZE) {
        printf("[FAIL] %s: block_size %zu exceeds buffer %d\n", tv->name, cipher->block_size, MAX_BLOCK_SIZE);
        return 1;
    }
    if (cipher->key_size > MAX_KEY_SIZE) {
        printf("[FAIL] %s: key_size %zu exceeds buffer %d\n", tv->name, cipher->key_size, MAX_KEY_SIZE);
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

    /* in-place (out == in) */
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
    const size_t num_vectors = sizeof(TEST_VECTORS) / sizeof(TEST_VECTORS[0]);
    int failures = 0;

    failures += check_sizes(&template_block_cipher);

    for (size_t i = 0; i < num_vectors; ++i) {
        failures += run_test_vector(&template_block_cipher, &TEST_VECTORS[i]);
    }

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
