#include "crypto-primitives/hight-lut.h"
#include "crypto-primitives/hight.h"
#include "kat-common.h"

static hight_ctx ctx_hight;
static hight_lut_ctx ctx_hight_lut;

static const kat_block_cipher_entry ENTRIES[] = {
    {"hight", &hight_block_cipher, &ctx_hight},
    {"hight-lut", &hight_lut_block_cipher, &ctx_hight_lut},
};

static const char *const FILES[] = {
    "ECBVarKey128.rsp",
    "ECBVarTxt128.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "HIGHT KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
