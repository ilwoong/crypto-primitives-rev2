#include "crypto-primitives/aria-lut.h"
#include "crypto-primitives/aria.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 16
#define MAX_KEY_SIZE 32
#define MAX_LINE 512

typedef struct {
    const char *name;
    const block_cipher *cipher128;
    const block_cipher *cipher192;
    const block_cipher *cipher256;
} aria_variant;

static const aria_variant VARIANTS[] = {
    {"aria", &aria128_block_cipher, &aria192_block_cipher, &aria256_block_cipher},
    {"aria-lut", &aria128_lut_block_cipher, &aria192_lut_block_cipher, &aria256_lut_block_cipher},
};

#define NUM_VARIANTS (sizeof(VARIANTS) / sizeof(VARIANTS[0]))

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static size_t hex_decode(uint8_t *out, const char *hex, size_t max_bytes)
{
    size_t len = strlen(hex);
    while (len > 0 && isspace((unsigned char)hex[len - 1]))
        len--;

    size_t nbytes = len / 2;
    if (nbytes > max_bytes)
        nbytes = max_bytes;

    for (size_t i = 0; i < nbytes; ++i) {
        int hi = hex_digit(hex[2 * i]);
        int lo = hex_digit(hex[2 * i + 1]);
        if (hi < 0 || lo < 0)
            return 0;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return nbytes;
}

static const char *parse_value(const char *line, const char *tag)
{
    size_t tag_len = strlen(tag);
    if (strncmp(line, tag, tag_len) != 0)
        return NULL;

    const char *p = line + tag_len;
    while (*p == ' ' || *p == '=')
        p++;
    return p;
}

static const block_cipher *select_cipher(const aria_variant *v, size_t key_bytes)
{
    switch (key_bytes) {
    case 16:
        return v->cipher128;
    case 24:
        return v->cipher192;
    case 32:
        return v->cipher256;
    default:
        return NULL;
    }
}

typedef enum { MODE_ENCRYPT, MODE_DECRYPT } test_mode;

static int run_vector(const aria_variant *v, test_mode mode, size_t count, const uint8_t *key, size_t key_len,
                      const uint8_t *plaintext, const uint8_t *ciphertext)
{
    const block_cipher *cipher = select_cipher(v, key_len);
    if (!cipher) {
        printf("[FAIL] %s: unsupported key length %zu at COUNT=%zu\n", v->name, key_len, count);
        return 1;
    }

    union {
        aria_ctx ref;
        aria_lut_ctx lut;
    } ctx;
    uint8_t result[BLOCK_SIZE];

    cipher->expand_key(&ctx, key);

    if (mode == MODE_ENCRYPT) {
        cipher->encrypt(&ctx, result, plaintext);
        if (memcmp(result, ciphertext, BLOCK_SIZE) != 0) {
            printf("[FAIL] %s encrypt COUNT=%zu\n", v->name, count);
            return 1;
        }
    }
    else {
        cipher->decrypt(&ctx, result, ciphertext);
        if (memcmp(result, plaintext, BLOCK_SIZE) != 0) {
            printf("[FAIL] %s decrypt COUNT=%zu\n", v->name, count);
            return 1;
        }
    }
    return 0;
}

static int process_file(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("[FAIL] Cannot open %s\n", filepath);
        return 1;
    }

    char line[MAX_LINE];
    test_mode mode = MODE_ENCRYPT;
    uint8_t key[MAX_KEY_SIZE];
    uint8_t pt[BLOCK_SIZE];
    uint8_t ct[BLOCK_SIZE];
    size_t key_len = 0;
    size_t count = 0;
    int has_key = 0, has_pt = 0, has_ct = 0;
    int failures = 0;
    int vectors = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "[ENCRYPT]"))
            mode = MODE_ENCRYPT;
        else if (strstr(line, "[DECRYPT]"))
            mode = MODE_DECRYPT;

        const char *val;
        if ((val = parse_value(line, "COUNT"))) {
            count = (size_t)atol(val);
            has_key = has_pt = has_ct = 0;
        }
        else if ((val = parse_value(line, "KEY"))) {
            key_len = hex_decode(key, val, MAX_KEY_SIZE);
            has_key = 1;
        }
        else if ((val = parse_value(line, "PLAINTEXT"))) {
            hex_decode(pt, val, BLOCK_SIZE);
            has_pt = 1;
        }
        else if ((val = parse_value(line, "CIPHERTEXT"))) {
            hex_decode(ct, val, BLOCK_SIZE);
            has_ct = 1;
        }

        if (has_key && has_pt && has_ct) {
            for (size_t v = 0; v < NUM_VARIANTS; ++v)
                failures += run_vector(&VARIANTS[v], mode, count, key, key_len, pt, ct);
            vectors++;
            has_key = has_pt = has_ct = 0;
        }
    }

    fclose(fp);
    printf("  %s: %d vectors x %zu variants", filepath, vectors, NUM_VARIANTS);
    if (failures > 0)
        printf(" (%d FAILURES)", failures);
    printf("\n");
    return failures;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <vectors_dir>\n", argv[0]);
        return 1;
    }

    const char *dir = argv[1];

    const char *files[] = {
        "ECBVarKey128.rsp", "ECBVarKey192.rsp", "ECBVarKey256.rsp",
        "ECBVarTxt128.rsp", "ECBVarTxt192.rsp", "ECBVarTxt256.rsp",
    };
    size_t nfiles = sizeof(files) / sizeof(files[0]);

    int total_failures = 0;
    printf("ARIA KAT (Known Answer Test)\n");

    for (size_t i = 0; i < nfiles; ++i) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
        total_failures += process_file(path);
    }

    printf("\n%d total failure(s)\n", total_failures);
    return total_failures == 0 ? 0 : 1;
}
