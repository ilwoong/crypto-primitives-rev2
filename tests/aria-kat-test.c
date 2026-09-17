#include "crypto-primitives/aria-lut.h"
#include "crypto-primitives/aria.h"
#include "kat-common.h"

static aria_ctx ctx_aria;
static aria_lut_ctx ctx_aria_lut;

static const kat_block_cipher_entry ENTRIES[] = {
    {"aria", &aria128_block_cipher, &ctx_aria},
    {"aria", &aria192_block_cipher, &ctx_aria},
    {"aria", &aria256_block_cipher, &ctx_aria},
    {"aria-lut", &aria128_lut_block_cipher, &ctx_aria_lut},
    {"aria-lut", &aria192_lut_block_cipher, &ctx_aria_lut},
    {"aria-lut", &aria256_lut_block_cipher, &ctx_aria_lut},
};

static const char *const FILES[] = {
    "ECBVarKey128.rsp", "ECBVarKey192.rsp", "ECBVarKey256.rsp",
    "ECBVarTxt128.rsp", "ECBVarTxt192.rsp", "ECBVarTxt256.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "ARIA KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
