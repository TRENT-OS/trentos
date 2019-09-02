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

#include "SeosKeyStore.h"
#include "SeosKeyStoreClient.h"

#include "KeyStoreInit.h"

#include "testSignatureRsa.h"
#include "testCrypto.h"
#include "testKeyStore.h"

#include <camkes.h>

/* Defines -------------------------------------------------------------------*/
#define NVM_CHANNEL_NUMBER      (6)
#define KEY_STORE_INSTANCE_NAME "KeyStore1"


/**
 * @weakgroup CryptoApi_test_scenarios
 * @{
 *
 * @brief Top level test runner
 *
 * @test TestCrypto_scenario_1      Perform Crypto test cases for the local and remote version of the crypto api
 *
 * @test TestKeyStore_scenario_1    Perform TestKeyStore_testCase_01 - 05 for a local version of the KeyStore
 *                                  using key generation
 *
 * @test TestKeyStore_scenario_2    Perform TestKeyStore_testCase_01 - 05 for a local version of the KeyStore
 *                                  using key import
 *
 * @test TestKeyStore_scenario_3    Perform TestKeyStore_testCase_01 - 05 for a remote version of the KeyStore
 *                                  using key generation
 *
 * @test TestKeyStore_scenario_4    Perform TestKeyStore_testCase_01 - 05 for a remote version of the KeyStore
 *                                  using key import
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

    err = SeosCrypto_init(&cryptoCtx, malloc, free, NULL, NULL);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    apiLocal    = SeosCrypto_TO_SEOS_CRYPTO_CTX(&cryptoCtx);
    apiRpc      = SeosCryptoClient_TO_SEOS_CRYPTO_CTX(&client);

    testRNG(apiLocal);
    testRNG(apiRpc);

    testDigestMD5(apiLocal);
    testDigestMD5(apiRpc);

    testDigestSHA256(apiLocal);
    testDigestSHA256(apiRpc);

    testCipherAES(apiLocal);
    testCipherAES(apiRpc);

    testSignatureRSA(&client);

    /***************************** KeyStore test *******************************/
    KeyStoreContext keyStoreCtx;
    SeosKeyStore localKeyStore;
    SeosKeyStoreClient keyStoreClient;
    SeosKeyStoreRpc_Handle keyStoreRpcHandle = NULL;

    /************************** Init local version ****************************/
    if (!keyStoreContext_ctor(&keyStoreCtx, NVM_CHANNEL_NUMBER,
                              (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }
    err = SeosKeyStore_init(&localKeyStore,
                            keyStoreCtx.fileStreamFactory,
                            &cryptoCtx,
                            KEY_STORE_INSTANCE_NAME);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_init failed with error code %d!", __func__,
                        err);
        return false;
    }

    /************************* Init remote version ***************************/
    err = KeyStore_getRpcHandle(&keyStoreRpcHandle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: KeyStore_getRpcHandle failed with error code %d!",
                        __func__,
                        err);
        return 0;
    }
    err = SeosKeyStoreClient_init(&keyStoreClient,
                                  keyStoreRpcHandle,
                                  keyStoreClientDataport);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreClient_init failed with error code %d!",
                        __func__,
                        err);
        return 0;
    }

    /******************** Test local and remote versions **********************/
    Debug_LOG_INFO("\n\n\n\n**************************** Starting 'TestKeyStore_scenario_1' ****************************\n");
    if (!testKeyStore(&(localKeyStore.parent), apiLocal, true))
    {
        Debug_LOG_ERROR("\n\nTestKeyStore_scenario_1 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTestKeyStore_scenario_1 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'TestKeyStore_scenario_2' ****************************\n");
    if (!testKeyStore(&(localKeyStore.parent), apiLocal, false))
    {
        Debug_LOG_ERROR("\n\nTestKeyStore_scenario_2 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTestKeyStore_scenario_2 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'TestKeyStore_scenario_3' ****************************\n");
    if (!testKeyStore(&(keyStoreClient.parent), apiRpc, true))
    {
        Debug_LOG_ERROR("\n\nTestKeyStore_scenario_3 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTestKeyStore_scenario_3 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'TestKeyStore_scenario_4' ****************************\n");
    if (!testKeyStore(&(keyStoreClient.parent), apiRpc, false))
    {
        Debug_LOG_ERROR("\n\nTestKeyStore_scenario_4 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTestKeyStore_scenario_4 succeeded!\n\n\n\n");
    }

    /***************************** Destruction *******************************/
    SeosKeyStore_deInit(&(localKeyStore.parent));
    keyStoreContext_dtor(&keyStoreCtx);
    SeosKeyStoreClient_deInit(&(keyStoreClient.parent));

    return 0;
}
///@}
