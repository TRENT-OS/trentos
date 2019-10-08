/**
 * @addtogroup CryptoApi_Tests
 * @{
 *
 * @file testRunner.c
 *
 * @brief top level test for the crypto API and the key store API
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "LibDebug/Debug.h"

#include "SeosCrypto.h"

#include "testSignatureRsa.h"
#include "testAgreement.h"
#include "testCrypto.h"
#include "testKey.h"

#include <camkes.h>
#include <string.h>

/* Defines -------------------------------------------------------------------*/

int entropyFunc(void*           ctx,
                unsigned char*  buf,
                size_t          len)
{
    // This would be the platform specific function to obtain entropy
    memset(buf, 0, len);
    return 0;
}

/**
 * @weakgroup CryptoApi_test_scenarios
 * @{
 *
 * @brief Top level test runner
 *
 * @test TestCrypto_scenario_1      Perform Crypto test cases for the local and remote version of the crypto api
 *
 *
 * @}
 */
int run()
{
    SeosCrypto cryptoCtx;
    SeosCryptoClient client;
    SeosCryptoCtx* apiLocal;
    SeosCryptoCtx* apiRpc;
    SeosCryptoRpc_Handle rpcHandle = NULL;
    seos_err_t err = SEOS_ERROR_GENERIC;

    err = Crypto_getRpcHandle(&rpcHandle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_LOG_INFO("%s: got rpc object %p from server", __func__, rpcHandle);

    err = SeosCryptoClient_init(&client, rpcHandle, cryptoClientDataport);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCrypto_init(&cryptoCtx, malloc, free, entropyFunc, NULL);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    apiLocal    = SeosCrypto_TO_SEOS_CRYPTO_CTX(&cryptoCtx);
    apiRpc      = SeosCryptoClient_TO_SEOS_CRYPTO_CTX(&client);

    testRNG(apiLocal);
    testRNG(apiRpc);

    testKey(apiLocal);
    testKey(apiRpc);

    testDigestMD5(apiLocal);
    testDigestMD5(apiRpc);

    testDigestSHA256(apiLocal);
    testDigestSHA256(apiRpc);

    testCipherAES_ECB(apiLocal);
    testCipherAES_ECB(apiRpc);

    testCipherAES_GCM(apiLocal);
    testCipherAES_GCM(apiRpc);

    testSignatureRSA(apiLocal);

    testAgreementDH(apiLocal);
    testAgreementECDH(apiLocal);

    return 0;
}
///@}
