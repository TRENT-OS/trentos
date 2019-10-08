/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */

#include "testKey.h"
#include "SeosCryptoApi.h"

#include <string.h>

static const SeosCryptoKey_RSAPub rsa1024 =
{
    .nBytes = {
        0x92, 0x22, 0xa2, 0x1b, 0x01, 0x61, 0xff, 0xc3, 0xdd, 0xc0, 0x4f, 0x8e, 0x91, 0xf1, 0xcc, 0x1f,
        0xdc, 0x0d, 0x2a, 0x08, 0x66, 0xaf, 0x0d, 0xd9, 0x05, 0xe8, 0xe7, 0xd6, 0x52, 0xa0, 0x38, 0x62,
        0x0a, 0x01, 0x8d, 0xd1, 0x3d, 0x43, 0x40, 0x6d, 0xfc, 0xf7, 0xc0, 0xa2, 0x1c, 0x87, 0xa5, 0x41,
        0xfe, 0xde, 0xcb, 0x73, 0x28, 0x5b, 0xbe, 0xd0, 0x4b, 0x9e, 0x3e, 0x59, 0xaf, 0x2f, 0x59, 0x92,
        0x22, 0x88, 0xf3, 0x00, 0x92, 0x66, 0x8d, 0xfc, 0x89, 0x99, 0x44, 0x38, 0x3c, 0xe4, 0x11, 0x42,
        0xd2, 0xa0, 0x95, 0xcc, 0xf1, 0xa8, 0x97, 0xe3, 0x71, 0x9d, 0xc1, 0xbe, 0x88, 0x68, 0x26, 0x42,
        0x2f, 0xe0, 0x10, 0x5e, 0x3e, 0xf6, 0xb2, 0xab, 0x0a, 0xa0, 0xe7, 0x87, 0xbd, 0xa4, 0x70, 0xdf,
        0x04, 0xce, 0x67, 0x6c, 0x48, 0xd3, 0xd3, 0xc0, 0x2d, 0xb2, 0x3f, 0xb3, 0x0d, 0x9c, 0xb0, 0xa1
    },
    .nLen = 128,
    .eBytes = {
        0x01, 0x00, 0x01
    },
    .eLen = 3,
};
static const SeosCryptoKey_AES aes128 =
{
    "0123456789abcdef", 16
};
static const SeosCryptoKey_AES aes192 =
{
    "0123456789abcdef01234567", 24
};
static const SeosCryptoKey_AES aes256 =
{
    "0123456789abcdef0123456789abcdef", 32
};
static const SeosCryptoKey_AES aes120 =
{
    "0123456789abcde", 15
};

static void
testKey_init_ok(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    // Try out 128 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try 192 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 192);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 256 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 1024 RSA bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 1024 RSA bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 1024 DH bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_DH_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 1024 DH bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_DH_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out a SECP256r1 prv key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_SECP256R1_PRV,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out a SECP256r1 pub key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_SECP256R1_PUB,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 128 bit key bit with exporting flags set
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW |
                                SeosCryptoKey_Flags_EXPORTABLE_WRAPPED,
                                128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_init_fail(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    // Try some strange keysizes
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES, 0, 666);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PUB, 0, 9999);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_SECP256R1_PRV, 0,
                                255);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    // mbedtls doesnt like RSA < 128 bits
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PUB, 0, 11);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);

    // Mess with alorithm ID
    err = SeosCryptoApi_keyInit(ctx, &key, 100, 0, 128);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_NONE, 0, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);

    // Mess with key handle
    err = SeosCryptoApi_keyInit(ctx, NULL, SeosCryptoKey_Type_RSA_PUB, 0, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_export_ok(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;
    size_t readLen;
    SeosCryptoKey_AES aesKey;
    void* pKey = &aesKey;

    // Export key that can be exported to our own buffer
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey);
    err = SeosCryptoApi_keyExport(ctx, key, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    Debug_ASSERT(memcmp(aesKey.bytes, aes128.bytes, 16) == 0);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Export key that can be exported into buffer passed from API
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = 0;
    pKey = NULL;
    err = SeosCryptoApi_keyExport(ctx, key, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    Debug_ASSERT(readLen == sizeof(aesKey));
    Debug_ASSERT(memcmp(aesKey.bytes, aes128.bytes, 16) == 0);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_export_fail(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;
    size_t readLen;
    SeosCryptoKey_AES aesKey;
    void* pKey = &aesKey;

    // Try non-exportable key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey);
    err = SeosCryptoApi_keyExport(ctx, key, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_ACCESS_DENIED == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try exportable key, but with no buffer given
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey);
    err = SeosCryptoApi_keyExport(ctx, key, NULL, NULL, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try exportable key, but with too small buffer
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey) - 5;
    err = SeosCryptoApi_keyExport(ctx, key, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_BUFFER_TOO_SMALL == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try exportable key, but give no keyhandle
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey);
    err = SeosCryptoApi_keyExport(ctx, NULL, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_HANDLE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try exporting an empty key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_EXPORTABLE_RAW, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    readLen = sizeof(aesKey);
    err = SeosCryptoApi_keyExport(ctx, key, NULL, &pKey, &readLen);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_FOUND == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_import_ok(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    // Try out 128 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 192 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 192);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes192, sizeof(aes192));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try out 256 bit key
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes256, sizeof(aes256));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Import RSA key with correct size
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &rsa1024, sizeof(rsa1024));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_import_fail(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    // Import wrong keysize
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes120, sizeof(aes120));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Import with with insufficiently big modulus
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 2048);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &rsa1024, sizeof(rsa1024));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Import with empty buffer
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, NULL, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Import without keyhandle
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, NULL, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_HANDLE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Import with wrapping key (currently not supported)
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, key, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Try importing a key, twice
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INSUFFICIENT_SPACE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_generate_ok(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGenerate(ctx, key);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_generate_fail(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle key;

    // Try generating symmetric key with wrong keytypes
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGenerate(ctx, key);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Don't pass a key handle
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGenerate(ctx, NULL);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_HANDLE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    // Generate on key that has already key material attached
    err = SeosCryptoApi_keyInit(ctx, &key, SeosCryptoKey_Type_AES,
                                SeosCryptoKey_Flags_NONE, 128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(ctx, key, NULL, &aes128, sizeof(aes128));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGenerate(ctx, key);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INSUFFICIENT_SPACE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, key);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_generatePair_ok(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle prvKey, pubKey;

    // Generate RSA keypair
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, prvKey, pubKey);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate EC keypair
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_SECP256R1_PRV,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_SECP256R1_PUB,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, prvKey, pubKey);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate DH keypair
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_DH_PRV,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_DH_PUB,
                                SeosCryptoKey_Flags_NONE, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, prvKey, pubKey);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    Debug_PRINTF("%s: OK\n", __func__);
}

static void
testKey_generatePair_fail(SeosCryptoCtx* ctx)
{
    seos_err_t err;
    SeosCrypto_KeyHandle prvKey, pubKey;

    // Generate with missing handles
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, NULL, pubKey);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_HANDLE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate with missing handles
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, prvKey, NULL);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_HANDLE == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate with mismatching key types
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_DH_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, prvKey, pubKey);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_INVALID_PARAMETER == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate with mismatching inverted types
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PRV,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, pubKey, prvKey);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    // Generate with both types being private
    err = SeosCryptoApi_keyInit(ctx, &prvKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(ctx, &pubKey, SeosCryptoKey_Type_RSA_PUB,
                                SeosCryptoKey_Flags_NONE, 1024);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyGeneratePair(ctx, pubKey, prvKey);
    Debug_ASSERT_PRINTFLN(SEOS_ERROR_NOT_SUPPORTED == err, "err %d", err);
    SeosCryptoApi_keyDeInit(ctx, prvKey);
    SeosCryptoApi_keyDeInit(ctx, pubKey);

    Debug_PRINTF("%s: OK\n", __func__);
}

void testKey(SeosCryptoCtx* ctx)
{
    testKey_init_ok(ctx);
    testKey_init_fail(ctx);

    testKey_export_ok(ctx);
    testKey_export_fail(ctx);

    testKey_import_ok(ctx);
    testKey_import_fail(ctx);

    testKey_generate_ok(ctx);
    testKey_generate_fail(ctx);

    testKey_generatePair_ok(ctx);
    testKey_generatePair_fail(ctx);
}