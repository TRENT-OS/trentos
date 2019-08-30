/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include "SeosCryptoAgreement.h"
#include "SeosCryptoClient.h"

#include "mbedtls/dhm.h"

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

static void
agreeOnKey(SeosCryptoRng* rng,
           SeosCryptoKey* private,
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
    err = SeosCryptoAgreement_init(&ctx, algo, private, rng);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    // We have received a public key (e.g., from a server) and use this to derive a secret
    // key of a given length
    err = SeosCryptoAgreement_computeShared(&ctx, public, buf, bufSize, outLen);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    SeosCryptoAgreement_deInit(&ctx);
}

static void
setupRng(seos_rng_t*    rng,
         SeosCryptoRng* cryptoRng)
{
    seos_err_t err;

    // Set up RNG
    err = seos_rng_init(rng, SeosCrypto_RANDOM_SEED_STR,
                        sizeof(SeosCrypto_RANDOM_SEED_STR) - 1 );
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoRng_init(cryptoRng, rng,
                             (SeosCryptoRng_ImplRngFunc) seos_rng_get_prng_bytes);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
}

void
testAgreementDH(SeosCryptoClient* client)
{
    SeosCryptoKey serverPublic, clientPrivate;
    mbedtls_dhm_context clientPrivateCtx, serverPublicCtx;
    unsigned char clientShared[32];
    SeosCryptoRng cryptoRng;
    seos_rng_t seosRng;
    seos_err_t err;
    int i;
    size_t n;

    setupRng(&seosRng, &cryptoRng);

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
    agreeOnKey(&cryptoRng, &clientPrivate, &serverPublic,
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