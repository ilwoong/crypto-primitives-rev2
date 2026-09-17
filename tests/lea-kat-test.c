#include "crypto-primitives/lea-unrolled.h"
#include "crypto-primitives/lea.h"
#include "kat-common.h"

static lea_ctx ctx_lea;
static lea_unrolled_ctx ctx_lea_unrolled;

static const kat_block_cipher_entry ENTRIES[] = {
    {"lea", &lea128_block_cipher, 16, 16, &ctx_lea},
    {"lea", &lea192_block_cipher, 24, 16, &ctx_lea},
    {"lea", &lea256_block_cipher, 32, 16, &ctx_lea},
    {"lea-unrolled", &lea128_unrolled_block_cipher, 16, 16, &ctx_lea_unrolled},
    {"lea-unrolled", &lea192_unrolled_block_cipher, 24, 16, &ctx_lea_unrolled},
    {"lea-unrolled", &lea256_unrolled_block_cipher, 32, 16, &ctx_lea_unrolled},
};

static const char *const FILES[] = {
    "ECBVarKey128.rsp", "ECBVarKey192.rsp", "ECBVarKey256.rsp",
    "ECBVarTxt128.rsp", "ECBVarTxt192.rsp", "ECBVarTxt256.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "LEA KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
