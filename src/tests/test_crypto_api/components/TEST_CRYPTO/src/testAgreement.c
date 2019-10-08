/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include "SeosCryptoAgreement.h"
#include "SeosCryptoApi.h"

#include <string.h>

// From mbedtls test suite; has very small keylength, do not use for anything
// other than testing!!
static const SeosCryptoKey_DHPrv clientDHPrvData =
{
    .pBytes = {0x12, 0xdf, 0x4d, 0x76, 0x89, 0xdf, 0xf4, 0xc9, 0x9d, 0x9a, 0xe5, 0x7d, 0x07},
    .pLen   = 13,
    .gBytes = {0x00, 0x1e, 0x32, 0x15, 0x8a, 0x35, 0xe3, 0x4d, 0x7b, 0x61, 0x96, 0x57, 0xd6},
    .gLen   = 13,
    .xBytes = {0x11, 0x46, 0xbc, 0x69, 0xaf, 0x6c, 0x32, 0xca, 0xfa, 0xd1, 0x46, 0xbc, 0x69},
    .xLen   = 13,
};
static const SeosCryptoKey_DHPub serverDHPubData =
{
    .pBytes  = {0x12, 0xdf, 0x4d, 0x76, 0x89, 0xdf, 0xf4, 0xc9, 0x9d, 0x9a, 0xe5, 0x7d, 0x07},
    .pLen    = 13,
    .gBytes  = {0x00, 0x1e, 0x32, 0x15, 0x8a, 0x35, 0xe3, 0x4d, 0x7b, 0x61, 0x96, 0x57, 0xd6},
    .gLen    = 13,
    .gxBytes = {0x00, 0x41, 0x83, 0xa8, 0x6f, 0x35, 0x71, 0x0e, 0x4e, 0x69, 0xb1, 0x64, 0xa4},
    .gxLen   = 13,
};
static const unsigned char dhSharedResult[] =
{
    0x0a, 0x01, 0xa3, 0x91, 0x9f, 0x2b, 0xa3, 0x69, 0xdc, 0x5b, 0x11, 0xde, 0x2c
};

// From mbedtls test suite; only one curve supported right now
static const SeosCryptoKey_SECP256r1Prv clientEcPrvData =
{
    .dBytes = {
        0xc6, 0xef, 0x9c, 0x5d, 0x78, 0xae, 0x01, 0x2a, 0x01, 0x11, 0x64, 0xac, 0xb3, 0x97, 0xce, 0x20,
        0x88, 0x68, 0x5d, 0x8f, 0x06, 0xbf, 0x9b, 0xe0, 0xb2, 0x83, 0xab, 0x46, 0x47, 0x6b, 0xee, 0x53
    },
    .dLen   = 32,
};
static const SeosCryptoKey_SECP256r1Pub serverEcPubData =
{
    .qxBytes = {
        0xda, 0xd0, 0xb6, 0x53, 0x94, 0x22, 0x1c, 0xf9, 0xb0, 0x51, 0xe1, 0xfe, 0xca, 0x57, 0x87, 0xd0,
        0x98, 0xdf, 0xe6, 0x37, 0xfc, 0x90, 0xb9, 0xef, 0x94, 0x5d, 0x0c, 0x37, 0x72, 0x58, 0x11, 0x80
    },
    .qxLen   = 32,
    .qyBytes = {
        0x52, 0x71, 0xa0, 0x46, 0x1c, 0xdb, 0x82, 0x52, 0xd6, 0x1f, 0x1c, 0x45, 0x6f, 0xa3, 0xe5, 0x9a,
        0xb1, 0xf4, 0x5b, 0x33, 0xac, 0xcf, 0x5f, 0x58, 0x38, 0x9e, 0x05, 0x77, 0xb8, 0x99, 0x0b, 0xb3
    },
    .qyLen   = 32,
};
static const unsigned char ecdhSharedResult[] =
{
    0xd6, 0x84, 0x0f, 0x6b, 0x42, 0xf6, 0xed, 0xaf, 0xd1, 0x31, 0x16, 0xe0, 0xe1, 0x25, 0x65, 0x20,
    0x2f, 0xef, 0x8e, 0x9e, 0xce, 0x7d, 0xce, 0x03, 0x81, 0x24, 0x64, 0xd0, 0x4b, 0x94, 0x42, 0xde
};
// Temporary use of the memIf, until we can use agreement through the API
static SeosCrypto_MemIf memIf =
{
    malloc,
    free
};

static void
agreeOnKey(SeosCryptoKey* private,
           SeosCryptoKey* public,
           unsigned int   algo,
           unsigned char* buf,
           size_t*        bufSize)
{
    SeosCryptoAgreement ctx;
    seos_err_t err;

    memset(buf, 0, *bufSize);

    // We have a private key (and a public one) and want to use to to agree on a shared
    // secret to perform symmetric cryptography
    err = SeosCryptoAgreement_init(&memIf, &ctx, algo, private);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // We have received a public key (e.g., from a server) and use this to derive a secret
    // key of a given length; for now, don't pass a RNG
    err = SeosCryptoAgreement_computeShared(&ctx, NULL, public, buf, bufSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    SeosCryptoAgreement_deInit(&memIf, &ctx);
}

void
testAgreementDH(SeosCryptoCtx* cryptoCtx)
{
    SeosCrypto_KeyHandle pubHandle, prvHandle;
    unsigned char clientShared[64];
    seos_err_t err;
    size_t i, n;

    err = SeosCryptoApi_keyInit(cryptoCtx, &pubHandle, SeosCryptoKey_Type_DH_PUB,
                                0, 101);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(cryptoCtx, &prvHandle,
                                SeosCryptoKey_Type_DH_PRV, 0, 101);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    err = SeosCryptoApi_keyImport(cryptoCtx, pubHandle, NULL, &serverDHPubData,
                                  sizeof(serverDHPubData));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(cryptoCtx, prvHandle, NULL, &clientDHPrvData,
                                  sizeof(clientDHPrvData));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    // Compute the side of the CLIENT
    n = sizeof(clientShared);
    agreeOnKey((SeosCryptoKey*) prvHandle, (SeosCryptoKey*) pubHandle,
               SeosCryptoAgreement_Algorithm_DH, clientShared, &n);
    Debug_PRINTF("Computed DH-shared secret for CLIENT:");
    for (i = 0; i < n; i++)
    {
        Debug_PRINTF(" 0x%02x", clientShared[i]);
    }
    Debug_PRINTF("\n");

    // // Make sure both actually match!
    Debug_ASSERT_PRINTFLN(!memcmp(clientShared, dhSharedResult,
                                  sizeof(dhSharedResult)), "Shared key mismatch");
    Debug_PRINTF("Client and server agreed on the expected key via DH.\n");

    err = SeosCryptoApi_keyDeInit(cryptoCtx, pubHandle);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyDeInit(cryptoCtx, prvHandle);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
}

void
testAgreementECDH(SeosCryptoCtx* cryptoCtx)
{
    SeosCrypto_KeyHandle pubHandle, prvHandle;
    unsigned char clientShared[64];
    seos_err_t err;
    size_t i, n;

    err = SeosCryptoApi_keyInit(cryptoCtx, &pubHandle,
                                SeosCryptoKey_Type_SECP256R1_PUB, 0, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyInit(cryptoCtx, &prvHandle,
                                SeosCryptoKey_Type_SECP256R1_PRV, 0, 256);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    err = SeosCryptoApi_keyImport(cryptoCtx, pubHandle, NULL, &serverEcPubData,
                                  sizeof(serverEcPubData));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyImport(cryptoCtx, prvHandle, NULL, &clientEcPrvData,
                                  sizeof(clientEcPrvData));
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    // Compute the side of the CLIENT
    n = sizeof(clientShared);
    agreeOnKey((SeosCryptoKey*) prvHandle, (SeosCryptoKey*) pubHandle,
               SeosCryptoAgreement_Algorithm_ECDH, clientShared, &n);
    Debug_PRINTF("Computed ECDH-shared secret for CLIENT:");
    for (i = 0; i < n; i++)
    {
        Debug_PRINTF(" 0x%02x", clientShared[i]);
    }
    Debug_PRINTF("\n");

    // Make sure both actually match!
    Debug_ASSERT_PRINTFLN(!memcmp(clientShared, ecdhSharedResult,
                                  sizeof(ecdhSharedResult)), "Shared key mismatch");
    Debug_PRINTF("Client and server agreed on the expected key via ECDH.\n");

    err = SeosCryptoApi_keyDeInit(cryptoCtx, pubHandle);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
    err = SeosCryptoApi_keyDeInit(cryptoCtx, prvHandle);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);
}