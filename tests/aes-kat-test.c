#include "crypto-primitives/aes-lut1.h"
#include "crypto-primitives/aes-lut2.h"
#include "crypto-primitives/aes-lut3.h"
#include "crypto-primitives/aes.h"
#include "kat-common.h"

static aes_ctx ctx_aes;
static aes_lut1_ctx ctx_aes_lut1;
static aes_lut2_ctx ctx_aes_lut2;
static aes_lut3_ctx ctx_aes_lut3;

static const kat_block_cipher_entry ENTRIES[] = {
    {"aes", &aes128_block_cipher, &ctx_aes},
    {"aes", &aes192_block_cipher, &ctx_aes},
    {"aes", &aes256_block_cipher, &ctx_aes},
    {"aes-lut1", &aes128_lut1_block_cipher, &ctx_aes_lut1},
    {"aes-lut1", &aes192_lut1_block_cipher, &ctx_aes_lut1},
    {"aes-lut1", &aes256_lut1_block_cipher, &ctx_aes_lut1},
    {"aes-lut2", &aes128_lut2_block_cipher, &ctx_aes_lut2},
    {"aes-lut2", &aes192_lut2_block_cipher, &ctx_aes_lut2},
    {"aes-lut2", &aes256_lut2_block_cipher, &ctx_aes_lut2},
    {"aes-lut3", &aes128_lut3_block_cipher, &ctx_aes_lut3},
    {"aes-lut3", &aes192_lut3_block_cipher, &ctx_aes_lut3},
    {"aes-lut3", &aes256_lut3_block_cipher, &ctx_aes_lut3},
};

static const char *const FILES[] = {
    "ECBVarKey128.rsp", "ECBVarKey192.rsp", "ECBVarKey256.rsp",
    "ECBVarTxt128.rsp", "ECBVarTxt192.rsp", "ECBVarTxt256.rsp",
};

int main(int argc, char *argv[])
{
    return kat_block_cipher_main(argc, argv, "AES KAT (Known Answer Test)", FILES, sizeof(FILES) / sizeof(FILES[0]),
                                 ENTRIES, sizeof(ENTRIES) / sizeof(ENTRIES[0]));
}
