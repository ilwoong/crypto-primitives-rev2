#include "crypto-primitives/seed.h"
#include "kat-common.h"

static seed_ctx ctx_seed;

static const kat_block_cipher_entry ENTRIES[] = {
    {"seed", &seed_block_cipher, 16, 16, &ctx_seed},
};

static const char *const FILES[] = {
    "ECBVarKey128.rsp",
    "ECBVarTxt128.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "SEED KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
