#include "crypto-primitives/lsh256.h"
#include "crypto-primitives/lsh512.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 1024
#define MAX_DIGEST_SIZE 64
#define MAX_LINE 4096

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

static const char *parse_value(const char *line, const char *tag)
{
    size_t tag_len = strlen(tag);
    if (strncmp(line, tag, tag_len) != 0 || line[tag_len] != ' ')
        return NULL;

    const char *p = line + tag_len;
    while (*p == ' ' || *p == '=')
        p++;
    return p;
}

typedef struct {
    const message_digest *md;
    void *ctx;
} digest_choice;

static digest_choice select_digest(size_t digest_len)
{
    static lsh256_ctx ctx256;
    static lsh512_ctx ctx512;

    if (digest_len == 32)
        return (digest_choice){&lsh256_message_digest, &ctx256};
    if (digest_len == 64)
        return (digest_choice){&lsh512_message_digest, &ctx512};
    return (digest_choice){NULL, NULL};
}

static int run_vector(size_t len_bits, const uint8_t *msg, size_t msg_len, const uint8_t *expected, size_t digest_len)
{
    if (len_bits != 8 * msg_len) {
        printf("[FAIL] Len = %zu does not match Msg length %zu bytes\n", len_bits, msg_len);
        return 1;
    }

    digest_choice dc = select_digest(digest_len);
    if (!dc.md) {
        printf("[FAIL] unsupported digest length %zu at Len = %zu\n", digest_len, len_bits);
        return 1;
    }

    uint8_t out[MAX_DIGEST_SIZE];
    int failures = 0;

    dc.md->init(dc.ctx);
    dc.md->update(dc.ctx, msg, msg_len);
    dc.md->final(dc.ctx, out);
    if (memcmp(out, expected, digest_len) != 0) {
        printf("[FAIL] single update Len = %zu\n", len_bits);
        failures++;
    }

    // Feed the message in uneven pieces so buffering across block boundaries is exercised too.
    dc.md->init(dc.ctx);
    for (size_t off = 0, piece = 1; off < msg_len; off += piece, piece = piece % 7 + 1) {
        size_t n = piece < msg_len - off ? piece : msg_len - off;
        dc.md->update(dc.ctx, msg + off, n);
    }
    dc.md->final(dc.ctx, out);
    if (memcmp(out, expected, digest_len) != 0) {
        printf("[FAIL] chunked update Len = %zu\n", len_bits);
        failures++;
    }

    return failures;
}

static int process_file(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("[FAIL] Cannot open %s\n", filepath);
        return 1;
    }

    static char line[MAX_LINE];
    static uint8_t msg[MAX_MSG_SIZE];
    uint8_t md[MAX_DIGEST_SIZE];
    size_t len_bits = 0, msg_len = 0, md_len = 0;
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
            msg_len = hex_decode(msg, val, MAX_MSG_SIZE);
            has_msg = 1;
        }
        else if ((val = parse_value(line, "MD"))) {
            md_len = hex_decode(md, val, MAX_DIGEST_SIZE);
            has_md = 1;
        }

        if (has_len && has_msg && has_md) {
            if (msg_len == (size_t)-1 || md_len == (size_t)-1) {
                printf("[FAIL] malformed hex at Len = %zu\n", len_bits);
                failures++;
            }
            else {
                failures += run_vector(len_bits, msg, msg_len, md, md_len);
            }
            vectors++;
            has_len = has_msg = has_md = 0;
        }
    }

    fclose(fp);
    printf("  %s: %d vectors", filepath, vectors);
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
        "LSH256-256ShortMsg.rsp",
        "LSH512-512ShortMsg.rsp",
    };
    size_t nfiles = sizeof(files) / sizeof(files[0]);

    int total_failures = 0;
    printf("LSH KAT (Known Answer Test)\n");

    for (size_t i = 0; i < nfiles; ++i) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
        total_failures += process_file(path);
    }

    printf("\n%d total failure(s)\n", total_failures);
    return total_failures == 0 ? 0 : 1;
}
