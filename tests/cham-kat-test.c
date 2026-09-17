#include "crypto-primitives/cham.h"
#include "kat-common.h"

static cham_ctx ctx_cham;

// CHAM-64/128 and CHAM-128/128 share a key length, so the block length disambiguates them.
static const kat_block_cipher_entry ENTRIES[] = {
    {"cham64_128", &cham64_128_block_cipher, &ctx_cham},
    {"cham128_128", &cham128_128_block_cipher, &ctx_cham},
    {"cham128_256", &cham128_256_block_cipher, &ctx_cham},
};

static const char *const FILES[] = {
    "ECBVarKey64_128.rsp",  "ECBVarTxt64_128.rsp",  "ECBVarKey128_128.rsp",
    "ECBVarTxt128_128.rsp", "ECBVarKey128_256.rsp", "ECBVarTxt128_256.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "CHAM KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
