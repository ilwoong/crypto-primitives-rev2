#include "crypto-primitives/lsh256.h"
#include "crypto-primitives/lsh512.h"
#include "kat-common.h"

static lsh256_ctx ctx_lsh256;
static lsh512_ctx ctx_lsh512;

static const kat_message_digest_entry ENTRIES[] = {
    {"lsh256", &lsh256_message_digest, &ctx_lsh256},
    {"lsh512", &lsh512_message_digest, &ctx_lsh512},
};

static const char *const FILES[] = {
    "LSH256-256ShortMsg.rsp",
    "LSH512-512ShortMsg.rsp",
};

int main(int argc, char *argv[])
{
    return kat_message_digest_main(argc, argv, "LSH KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                   ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
