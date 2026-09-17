#ifndef CRYPTO_PRIMITIVES_TESTS_KAT_COMMON_H
#define CRYPTO_PRIMITIVES_TESTS_KAT_COMMON_H

#include "crypto-primitives/cipher.h"
#include "crypto-primitives/message-digest.h"

#include <stddef.h>

#define KAT_MAX_KEY_SIZE 32
#define KAT_MAX_BLOCK_SIZE 16
#define KAT_MAX_MSG_SIZE 1024
#define KAT_MAX_DIGEST_SIZE 64
#define KAT_MAX_LINE 4096

// A vector runs on every entry whose key and block length match the vector.
typedef struct {
    const char *name;
    const block_cipher *cipher;
    void *ctx;
} kat_block_cipher_entry;

// A vector runs on every entry whose digest length matches the MD of the vector.
typedef struct {
    const char *name;
    const message_digest *md;
    void *ctx;
} kat_message_digest_entry;

int kat_block_cipher_main(int argc, char *argv[], const char *title, const char *const *files, size_t nfiles,
                          const kat_block_cipher_entry *entries, size_t nentries);

int kat_message_digest_main(int argc, char *argv[], const char *title, const char *const *files, size_t nfiles,
                            const kat_message_digest_entry *entries, size_t nentries);

#endif
