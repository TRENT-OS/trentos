/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include "SeosCryptoAgreement.h"
#include "SeosCryptoClient.h"

#include "mbedtls/dhm.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"

// DH-grop parameters; taken from mbedTLS test suite. These are not secure, so
// please do not use them for anything else..
static const unsigned char DH_PARAM_P[] =
{
    0x12, 0xdf, 0x4d, 0x76, 0x89, 0xdf, 0xf4, 0xc9, 0x9d, 0x9a, 0xe5, 0x7d, 0x07
};
static const unsigned char DH_PARAM_G[] =
{
    0x00, 0x1e, 0x32, 0x15, 0x8a, 0x35, 0xe3, 0x4d, 0x7b, 0x61, 0x96, 0x57, 0xd6
};
// Client private parameter
static const unsigned char DH_CLIENT_PRIVATE[] =
{
    0x11, 0x46, 0xbc, 0x69, 0xaf, 0x6c, 0x32, 0xca, 0xfa, 0xd1, 0x46, 0xbc, 0x69
};
// Server public parameter
static const unsigned char DH_SERVER_PUBLIC[] =
{
    0x00, 0x41, 0x83, 0xa8, 0x6f, 0x35, 0x71, 0x0e, 0x4e, 0x69, 0xb1, 0x64, 0xa4
};
// The expected result
static const unsigned char DH_SHARED[] =
{
    0x0a, 0x01, 0xa3, 0x91, 0x9f, 0x2b, 0xa3, 0x69, 0xdc, 0x5b, 0x11, 0xde, 0x2c
};

// ECDH parameters for SECP256R1 taken from mbedTLS test suite. We only support
// the NIST curve SECP256R1 at this point.
static const unsigned int ECDH_CURVE_ID = MBEDTLS_ECP_DP_SECP256R1;
// Client's private key, a scalar
static const unsigned char ECDH_CLIENT_PRIVATE[] =
{
    0xc6, 0xef, 0x9c, 0x5d, 0x78, 0xae, 0x01, 0x2a, 0x01, 0x11, 0x64, 0xac, 0xb3, 0x97, 0xce, 0x20,
    0x88, 0x68, 0x5d, 0x8f, 0x06, 0xbf, 0x9b, 0xe0, 0xb2, 0x83, 0xab, 0x46, 0x47, 0x6b, 0xee, 0x53
};
// X and Y coordinates of servers public point
static const unsigned char ECDH_SERVER_PUBLIC_X[] =
{
    0xda, 0xd0, 0xb6, 0x53, 0x94, 0x22, 0x1c, 0xf9, 0xb0, 0x51, 0xe1, 0xfe, 0xca, 0x57, 0x87, 0xd0,
    0x98, 0xdf, 0xe6, 0x37, 0xfc, 0x90, 0xb9, 0xef, 0x94, 0x5d, 0x0c, 0x37, 0x72, 0x58, 0x11, 0x80
};
static const unsigned char ECDH_SERVER_PUBLIC_Y[] =
{
    0x52, 0x71, 0xa0, 0x46, 0x1c, 0xdb, 0x82, 0x52, 0xd6, 0x1f, 0x1c, 0x45, 0x6f, 0xa3, 0xe5, 0x9a,
    0xb1, 0xf4, 0x5b, 0x33, 0xac, 0xcf, 0x5f, 0x58, 0x38, 0x9e, 0x05, 0x77, 0xb8, 0x99, 0x0b, 0xb3
};
// The expected result
static const unsigned char ECDH_SHARED[] =
{
    0xd6, 0x84, 0x0f, 0x6b, 0x42, 0xf6, 0xed, 0xaf, 0xd1, 0x31, 0x16, 0xe0, 0xe1, 0x25, 0x65, 0x20,
    0x2f, 0xef, 0x8e, 0x9e, 0xce, 0x7d, 0xce, 0x03, 0x81, 0x24, 0x64, 0xd0, 0x4b, 0x94, 0x42, 0xde
};

static void
agreeOnKey(SeosCryptoKey* private,
           SeosCryptoKey* public,
           unsigned int   algo,
           unsigned char* buf,
           size_t         bufSize,
           size_t*        outLen)
{
    SeosCryptoAgreement ctx;
    seos_err_t err;

    memset(buf, 0, bufSize);

    // We have a private key (and a public one) and want to use to to agree on a shared
    // secret to perform symmetric cryptography
    err = SeosCryptoAgreement_init(&ctx, algo, private);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // We have received a public key (e.g., from a server) and use this to derive a secret
    // key of a given length; for now, don't pass a RNG
    err = SeosCryptoAgreement_computeShared(&ctx, NULL, public, buf, bufSize,
                                            outLen);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    SeosCryptoAgreement_deInit(&ctx);
}

void
testAgreementDH(SeosCryptoClient* client)
{
    SeosCryptoKey serverPublic, clientPrivate;
    mbedtls_dhm_context clientPrivateCtx, serverPublicCtx;
    unsigned char clientShared[32];
    seos_err_t err;
    int i;
    size_t n;

    // Create private CLIENT key
    mbedtls_dhm_init(&clientPrivateCtx);
    err = SeosCryptoKey_initDhPrivate(&clientPrivate, &clientPrivateCtx, DH_PARAM_P,
                                      sizeof(DH_PARAM_P), DH_PARAM_G, sizeof(DH_PARAM_G), DH_CLIENT_PRIVATE,
                                      sizeof(DH_CLIENT_PRIVATE));
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // Create public SERVER key
    mbedtls_dhm_init(&serverPublicCtx);
    err = SeosCryptoKey_initDhPublic(&serverPublic, &serverPublicCtx, DH_PARAM_P,
                                     sizeof(DH_PARAM_P), DH_PARAM_G, sizeof(DH_PARAM_G), DH_SERVER_PUBLIC,
                                     sizeof(DH_SERVER_PUBLIC));
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // Compute the side of the CLIENT
    agreeOnKey(&clientPrivate, &serverPublic,
               SeosCryptoAgreement_Algorithm_DH, clientShared, sizeof(clientShared), &n);
    Debug_PRINTF("Computed DH-shared secret for CLIENT:");
    for (i = 0; i < n; i++)
    {
        Debug_PRINTF(" 0x%02x", clientShared[i]);
    }
    Debug_PRINTF("\n");

    // Make sure both actually match!
    Debug_ASSERT_PRINTFLN(!memcmp(clientShared, DH_SHARED, sizeof(DH_SHARED)),
                          "Shared key mismatch");

    Debug_PRINTF("Client and server agreed on the expected key via DH.\n");

    mbedtls_dhm_free(&clientPrivateCtx);
    mbedtls_dhm_free(&serverPublicCtx);
}

void
testAgreementECDH(SeosCryptoClient* client)
{
    SeosCryptoKey clientPrivate;
    SeosCryptoKey serverPublic;
    mbedtls_ecp_keypair clientPrivateCtx, serverPublicCtx;
    unsigned char clientShared[32];
    seos_err_t err;
    int i;
    size_t n;

    // Create private CLIENT key
    mbedtls_ecp_keypair_init(&clientPrivateCtx);
    err = SeosCryptoKey_initEcdhPrivate(&clientPrivate, &clientPrivateCtx,
                                        ECDH_CURVE_ID, ECDH_CLIENT_PRIVATE, sizeof(ECDH_CLIENT_PRIVATE));
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // Create public SERVER key
    mbedtls_ecp_keypair_init(&serverPublicCtx);
    err = SeosCryptoKey_initEcdhPublic(&serverPublic, &serverPublicCtx,
                                       ECDH_CURVE_ID, ECDH_SERVER_PUBLIC_X, sizeof(ECDH_SERVER_PUBLIC_X),
                                       ECDH_SERVER_PUBLIC_Y, sizeof(ECDH_SERVER_PUBLIC_Y));
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // Compute the side of the CLIENT
    agreeOnKey(&clientPrivate, &serverPublic,
               SeosCryptoAgreement_Algorithm_ECDH, clientShared, sizeof(clientShared), &n);
    Debug_PRINTF("Computed ECDH-shared secret for CLIENT:");
    for (i = 0; i < n; i++)
    {
        Debug_PRINTF(" 0x%02x", clientShared[i]);
    }
    Debug_PRINTF("\n");

    // Make sure both actually match!
    Debug_ASSERT_PRINTFLN(!memcmp(clientShared, ECDH_SHARED, sizeof(ECDH_SHARED)),
                          "Shared key mismatch");

    Debug_PRINTF("Client and server agreed on the expected key via ECDH.\n");

    mbedtls_ecp_keypair_free(&serverPublicCtx);
    mbedtls_ecp_keypair_free(&clientPrivateCtx);
}