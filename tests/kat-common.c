#include "kat-common.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Returns the number of bytes decoded, or (size_t)-1 on a malformed or oversized value.
static size_t hex_decode(uint8_t *out, const char *hex, size_t max_bytes)
{
    size_t len = strlen(hex);
    while (len > 0 && isspace((unsigned char)hex[len - 1]))
        len--;

    if (len % 2 != 0 || len / 2 > max_bytes)
        return (size_t)-1;

    size_t nbytes = len / 2;
    for (size_t i = 0; i < nbytes; ++i) {
        int hi = hex_digit(hex[2 * i]);
        int lo = hex_digit(hex[2 * i + 1]);
        if (hi < 0 || lo < 0)
            return (size_t)-1;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return nbytes;
}

// The character after the tag must be a separator so that "Len" does not match "Length".
static const char *parse_value(const char *line, const char *tag)
{
    size_t tag_len = strlen(tag);
    if (strncmp(line, tag, tag_len) != 0 || (line[tag_len] != ' ' && line[tag_len] != '='))
        return NULL;

    const char *p = line + tag_len;
    while (*p == ' ' || *p == '=')
        p++;
    return p;
}

static void print_file_summary(const char *filepath, int vectors, size_t variants, int *failures)
{
    printf("  %s: %d vectors", filepath, vectors);
    if (variants > 1)
        printf(" x %zu variants", variants);
    if (vectors == 0) {
        printf(" (NO VECTORS)");
        (*failures)++;
    }
    if (*failures > 0)
        printf(" (%d FAILURES)", *failures);
    printf("\n");
}

static int run_files(int argc, char *argv[], const char *title, const char *const *files, size_t nfiles,
                     int (*process_file)(const char *, const void *, size_t), const void *entries, size_t nentries)
{
    if (argc < 2) {
        printf("Usage: %s <vectors_dir>\n", argv[0]);
        return 1;
    }

    const char *dir = argv[1];
    int total_failures = 0;
    printf("%s\n", title);

    for (size_t i = 0; i < nfiles; ++i) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
        total_failures += process_file(path, entries, nentries);
    }

    printf("\n%d total failure(s)\n", total_failures);
    return total_failures == 0 ? 0 : 1;
}

typedef enum { mode_encrypt, mode_decrypt } kat_mode;

static int run_block_cipher_vector(const kat_block_cipher_entry *entries, size_t nentries, kat_mode mode, size_t count,
                                   const uint8_t *key, size_t key_len, const uint8_t *plaintext,
                                   const uint8_t *ciphertext, size_t block_len, size_t *matched)
{
    int failures = 0;
    size_t nmatched = 0;
    uint8_t result[KAT_MAX_BLOCK_SIZE];

    for (size_t i = 0; i < nentries; ++i) {
        const kat_block_cipher_entry *entry = &entries[i];
        if (entry->cipher->key_size != key_len || entry->cipher->block_size != block_len)
            continue;
        ++nmatched;

        entry->cipher->expand_key(entry->ctx, key);
        if (mode == mode_encrypt) {
            entry->cipher->encrypt(entry->ctx, result, plaintext);
            if (memcmp(result, ciphertext, block_len) != 0) {
                printf("[FAIL] %s encrypt COUNT=%zu\n", entry->name, count);
                failures++;
            }
        }
        else {
            entry->cipher->decrypt(entry->ctx, result, ciphertext);
            if (memcmp(result, plaintext, block_len) != 0) {
                printf("[FAIL] %s decrypt COUNT=%zu\n", entry->name, count);
                failures++;
            }
        }
    }

    if (nmatched == 0) {
        printf("[FAIL] unsupported key/block length %zu/%zu at COUNT=%zu\n", key_len, block_len, count);
        failures++;
    }

    *matched = nmatched;
    return failures;
}

static int process_block_cipher_file(const char *filepath, const void *entries_arg, size_t nentries)
{
    const kat_block_cipher_entry *entries = entries_arg;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("[FAIL] Cannot open %s\n", filepath);
        return 1;
    }

    static char line[KAT_MAX_LINE];
    uint8_t key[KAT_MAX_KEY_SIZE];
    uint8_t pt[KAT_MAX_BLOCK_SIZE];
    uint8_t ct[KAT_MAX_BLOCK_SIZE];
    kat_mode mode = mode_encrypt;
    size_t key_len = 0, pt_len = 0, ct_len = 0;
    size_t count = 0;
    size_t variants = 0;
    int has_key = 0, has_pt = 0, has_ct = 0;
    int failures = 0;
    int vectors = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "[ENCRYPT]"))
            mode = mode_encrypt;
        else if (strstr(line, "[DECRYPT]"))
            mode = mode_decrypt;

        const char *val;
        if ((val = parse_value(line, "COUNT"))) {
            count = (size_t)strtoul(val, NULL, 10);
            has_key = has_pt = has_ct = 0;
        }
        else if ((val = parse_value(line, "KEY"))) {
            key_len = hex_decode(key, val, KAT_MAX_KEY_SIZE);
            has_key = 1;
        }
        else if ((val = parse_value(line, "PLAINTEXT"))) {
            pt_len = hex_decode(pt, val, KAT_MAX_BLOCK_SIZE);
            has_pt = 1;
        }
        else if ((val = parse_value(line, "CIPHERTEXT"))) {
            ct_len = hex_decode(ct, val, KAT_MAX_BLOCK_SIZE);
            has_ct = 1;
        }

        if (has_key && has_pt && has_ct) {
            if (key_len == (size_t)-1 || pt_len == (size_t)-1 || ct_len == (size_t)-1) {
                printf("[FAIL] malformed hex at COUNT=%zu\n", count);
                failures++;
            }
            else if (pt_len != ct_len) {
                printf("[FAIL] plaintext/ciphertext length mismatch at COUNT=%zu\n", count);
                failures++;
            }
            else {
                failures +=
                    run_block_cipher_vector(entries, nentries, mode, count, key, key_len, pt, ct, pt_len, &variants);
            }
            vectors++;
            has_key = has_pt = has_ct = 0;
        }
    }

    fclose(fp);
    print_file_summary(filepath, vectors, variants, &failures);
    return failures;
}

int kat_block_cipher_main(int argc, char *argv[], const char *title, const char *const *files, size_t nfiles,
                          const kat_block_cipher_entry *entries, size_t nentries)
{
    return run_files(argc, argv, title, files, nfiles, process_block_cipher_file, entries, nentries);
}

static int run_message_digest_vector(const kat_message_digest_entry *entries, size_t nentries, size_t len_bits,
                                     const uint8_t *msg, size_t msg_len, const uint8_t *expected, size_t digest_len,
                                     size_t *matched)
{
    if (len_bits != 8 * msg_len) {
        printf("[FAIL] Len = %zu does not match Msg length %zu bytes\n", len_bits, msg_len);
        return 1;
    }

    int failures = 0;
    size_t nmatched = 0;
    uint8_t out[KAT_MAX_DIGEST_SIZE];

    for (size_t i = 0; i < nentries; ++i) {
        const kat_message_digest_entry *entry = &entries[i];
        if (entry->md->digest_size != digest_len)
            continue;
        ++nmatched;

        entry->md->init(entry->ctx);
        entry->md->update(entry->ctx, msg, msg_len);
        entry->md->final(entry->ctx, out);
        if (memcmp(out, expected, digest_len) != 0) {
            printf("[FAIL] %s single update Len = %zu\n", entry->name, len_bits);
            failures++;
        }

        // Feed the message in uneven pieces so buffering across block boundaries is exercised too.
        entry->md->init(entry->ctx);
        for (size_t off = 0, piece = 1; off < msg_len; off += piece, piece = piece % 7 + 1) {
            size_t n = piece < msg_len - off ? piece : msg_len - off;
            entry->md->update(entry->ctx, msg + off, n);
        }
        entry->md->final(entry->ctx, out);
        if (memcmp(out, expected, digest_len) != 0) {
            printf("[FAIL] %s chunked update Len = %zu\n", entry->name, len_bits);
            failures++;
        }
    }

    if (nmatched == 0) {
        printf("[FAIL] unsupported digest length %zu at Len = %zu\n", digest_len, len_bits);
        failures++;
    }

    *matched = nmatched;
    return failures;
}

static int process_message_digest_file(const char *filepath, const void *entries_arg, size_t nentries)
{
    const kat_message_digest_entry *entries = entries_arg;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("[FAIL] Cannot open %s\n", filepath);
        return 1;
    }

    static char line[KAT_MAX_LINE];
    static uint8_t msg[KAT_MAX_MSG_SIZE];
    uint8_t md[KAT_MAX_DIGEST_SIZE];
    size_t len_bits = 0, msg_len = 0, md_len = 0;
    size_t variants = 0;
    int has_len = 0, has_msg = 0, has_md = 0;
    int failures = 0;
    int vectors = 0;

    while (fgets(line, sizeof(line), fp)) {
        const char *val;
        if ((val = parse_value(line, "Len"))) {
            len_bits = (size_t)strtoul(val, NULL, 10);
            has_len = 1;
            has_msg = has_md = 0;
        }
        else if ((val = parse_value(line, "Msg"))) {
            msg_len = hex_decode(msg, val, KAT_MAX_MSG_SIZE);
            has_msg = 1;
        }
        else if ((val = parse_value(line, "MD"))) {
            md_len = hex_decode(md, val, KAT_MAX_DIGEST_SIZE);
            has_md = 1;
        }

        if (has_len && has_msg && has_md) {
            if (msg_len == (size_t)-1 || md_len == (size_t)-1) {
                printf("[FAIL] malformed hex at Len = %zu\n", len_bits);
                failures++;
            }
            else {
                failures += run_message_digest_vector(entries, nentries, len_bits, msg, msg_len, md, md_len, &variants);
            }
            vectors++;
            has_len = has_msg = has_md = 0;
        }
    }

    fclose(fp);
    print_file_summary(filepath, vectors, variants, &failures);
    return failures;
}

int kat_message_digest_main(int argc, char *argv[], const char *title, const char *const *files, size_t nfiles,
                            const kat_message_digest_entry *entries, size_t nentries)
{
    return run_files(argc, argv, title, files, nfiles, process_message_digest_file, entries, nentries);
}
