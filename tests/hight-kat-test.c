#include "crypto-primitives/hight-lut.h"
#include "crypto-primitives/hight.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 8
#define KEY_SIZE 16
#define MAX_BLOCK_SIZE 16
#define MAX_KEY_SIZE 32
#define MAX_LINE 512

typedef struct {
    const char *name;
    const block_cipher *cipher;
} hight_variant;

static const hight_variant VARIANTS[] = {
    {"hight", &hight_block_cipher},
    {"hight-lut", &hight_lut_block_cipher},
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
        return 0;

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

typedef enum { MODE_ENCRYPT, MODE_DECRYPT } test_mode;

static int run_vector(const hight_variant *v, test_mode mode, size_t count, const uint8_t *key,
                      const uint8_t *plaintext, const uint8_t *ciphertext)
{
    union {
        hight_ctx ref;
        hight_lut_ctx lut;
    } ctx;
    uint8_t result[BLOCK_SIZE];

    v->cipher->expand_key(&ctx, key);

    if (mode == MODE_ENCRYPT) {
        v->cipher->encrypt(&ctx, result, plaintext);
        if (memcmp(result, ciphertext, BLOCK_SIZE) != 0) {
            printf("[FAIL] %s encrypt COUNT=%zu\n", v->name, count);
            return 1;
        }
    }
    else {
        v->cipher->decrypt(&ctx, result, ciphertext);
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
    uint8_t pt[MAX_BLOCK_SIZE];
    uint8_t ct[MAX_BLOCK_SIZE];
    size_t key_len = 0, pt_len = 0, ct_len = 0;
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
            pt_len = hex_decode(pt, val, MAX_BLOCK_SIZE);
            has_pt = 1;
        }
        else if ((val = parse_value(line, "CIPHERTEXT"))) {
            ct_len = hex_decode(ct, val, MAX_BLOCK_SIZE);
            has_ct = 1;
        }

        if (has_key && has_pt && has_ct) {
            if (key_len != KEY_SIZE || pt_len != BLOCK_SIZE || ct_len != BLOCK_SIZE) {
                printf("[FAIL] unexpected key/pt/ct length %zu/%zu/%zu at COUNT=%zu\n", key_len, pt_len, ct_len, count);
                failures++;
            }
            else {
                for (size_t v = 0; v < NUM_VARIANTS; ++v)
                    failures += run_vector(&VARIANTS[v], mode, count, key, pt, ct);
            }
            vectors++;
            has_key = has_pt = has_ct = 0;
        }
    }

    fclose(fp);
    printf("  %s: %d vectors x %zu variants", filepath, vectors, NUM_VARIANTS);
    if (vectors == 0) {
        printf(" (NO VECTORS)");
        failures++;
    }
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
        "ECBVarKey128.rsp",
        "ECBVarTxt128.rsp",
    };
    size_t nfiles = sizeof(files) / sizeof(files[0]);

    int total_failures = 0;
    printf("HIGHT KAT (Known Answer Test)\n");

    for (size_t i = 0; i < nfiles; ++i) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
        total_failures += process_file(path);
    }

    printf("\n%d total failure(s)\n", total_failures);
    return total_failures == 0 ? 0 : 1;
}
