/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
/* Includes ------------------------------------------------------------------*/
#include "ChanMux/ChanMuxClient.h"
#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "SeosKeyStore.h"
#include "SeosKeyStoreClient.h"
#include "SeosKeyStoreRpc.h"
#include "SeosKeyStoreCtx.h"
#include "camkes.h"

/* Defines -------------------------------------------------------------------*/
#define NVM_PARTITION_SIZE      (1024*128)
#define CHANMUX_NVM_CHANNEL_1   6
#define CHANMUX_NVM_CHANNEL_2   7

#define KEY_STORE_INSTANCE_NAME "KeyStore1"

#define MASTER_KEY_BYTES        "f131830db44c54742fc3f3265f0f1a0cf131830db44c54742fc3f3265f0f1a0c"
#define MASTER_KEY_NAME         "MasterKey"
#define MASTER_KEY_SIZE         64

#define GENERATED_KEY_NAME      "GeneratedKey"
#define GENERATED_KEY_SIZE      64

#define MAX_KEY_LEN             256

/* Private types ---------------------------------------------------------*/
typedef struct KeyStoreContext
{
    ProxyNVM proxyNVM;
    ChanMuxClient chanMuxClient;
    AesNvm aesNvm;
    SeosSpiffs fs;
    FileStreamFactory* fileStreamFactory;
    SeosCrypto cryptoCore;
    SeosKeyStore keyStore;
} KeyStoreContext;

/* Private functions prototypes ----------------------------------------------*/
bool KeyStoreContext_ctor(KeyStoreContext* keyStoreCtx, uint8_t channelNum,
                          void* dataport);
void KeyStoreContext_dtor(KeyStoreContext* keyStoreCtx);
seos_err_t testKeyDataRetreival(SeosKeyStoreCtx* keyStoreCtx,
                                const char* keyName,
                                size_t expectedKeySize,
                                const char* expectedKeyData);

/* Private variables ---------------------------------------------------------*/

/* Test ----------------------------------------------------------------------*/
int run()
{
    KeyStoreContext keyStoreCtx_1;

    if (!KeyStoreContext_ctor(&keyStoreCtx_1, CHANMUX_NVM_CHANNEL_1,
                              (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }

    SeosCryptoKey* masterKey;
    SeosCryptoKey* generatedKey;

    // import the created key
    seos_err_t err = SeosKeyStoreApi_importKey(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)),
                                               &masterKey,
                                               MASTER_KEY_NAME,
                                               MASTER_KEY_BYTES,
                                               SeosCryptoCipher_Algorithm_AES_CBC_ENC,
                                               1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                               MASTER_KEY_SIZE * 8);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_importKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe master key is succesfully imported!\n");

    // read the master key
    err = testKeyDataRetreival(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)),
                               MASTER_KEY_NAME,
                               MASTER_KEY_SIZE,
                               MASTER_KEY_BYTES);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testKeyDataRetreival failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe master key data is succesfully read!\n");

    // delete the master key
    err = SeosKeyStoreApi_deleteKey(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)), masterKey,
                                    MASTER_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_deleteKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    // check if the key is actaully deleted by verifying that the getKey results in an error
    err = testKeyDataRetreival(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)),
                               MASTER_KEY_NAME,
                               MASTER_KEY_SIZE,
                               MASTER_KEY_BYTES);
    if (err != SEOS_ERROR_NOT_FOUND)
    {
        Debug_LOG_ERROR("%s: Expected to receive a SEOS_ERROR_NOT_FOUND after reading the deleted key, but received an err code: %d! Exiting test...",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe master key is succesfully deleted!\n");

    // generate new key
    err = SeosKeyStoreApi_generateKey(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)),
                                      &generatedKey,
                                      GENERATED_KEY_NAME,
                                      SeosCryptoCipher_Algorithm_AES_CBC_ENC,
                                      1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                      GENERATED_KEY_SIZE * 8);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_generateKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    // read the generated key
    err = testKeyDataRetreival(&(SeosKeyStore_TO_SEOS_KEY_STORE_CTX(keyStoreCtx_1.keyStore)),
                               GENERATED_KEY_NAME,
                               GENERATED_KEY_SIZE,
                               generatedKey->bytes);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testKeyDataRetreival failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe generated key data is succesfully read!\n");

    KeyStoreContext_dtor(&keyStoreCtx_1);

    return 0;
}

/* Private functions ---------------------------------------------------------*/
bool KeyStoreContext_ctor(KeyStoreContext* keyStoreCtx, uint8_t channelNum,
                          void* dataport)
{
    if (!ChanMuxClient_ctor(&(keyStoreCtx->chanMuxClient), channelNum, dataport))
    {
        Debug_LOG_ERROR("%s: Failed to construct chanMuxClient, channel %d!", __func__,
                        channelNum);
        return false;
    }

    if (!ProxyNVM_ctor(&(keyStoreCtx->proxyNVM), &(keyStoreCtx->chanMuxClient),
                       dataport, PAGE_SIZE))
    {
        Debug_LOG_ERROR("%s: Failed to construct proxyNVM, channel %d!", __func__,
                        channelNum);
        return false;
    }

    if (!AesNvm_ctor(&(keyStoreCtx->aesNvm),
                     ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM))))
    {
        Debug_LOG_ERROR("%s: Failed to initialize AesNvm, channel %d!", __func__,
                        channelNum);
        return false;
    }

    if (!SeosSpiffs_ctor(&(keyStoreCtx->fs), AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)),
                         NVM_PARTITION_SIZE, 0))
    {
        Debug_LOG_ERROR("%s: Failed to initialize spiffs, channel %d!", __func__,
                        channelNum);
        return false;
    }

    seos_err_t ret = SeosSpiffs_mount(&(keyStoreCtx->fs));
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: spiffs mount failed with error code %d, channel %d!",
                        __func__, ret, channelNum);
        return false;
    }

    keyStoreCtx->fileStreamFactory = SpiffsFileStreamFactory_TO_FILE_STREAM_FACTORY(
                                         SpiffsFileStreamFactory_getInstance(&(keyStoreCtx->fs)));
    if (keyStoreCtx->fileStreamFactory == NULL)
    {
        Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance, channel %d!",
                        __func__, channelNum);
        return false;
    }
    ret = SeosCrypto_init(&(keyStoreCtx->cryptoCore), malloc, free, NULL, NULL);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCrypto_init failed with error code %d",
                        __func__, ret);
        return false;
    }

    ret = SeosKeyStore_init(&(keyStoreCtx->keyStore),
                           keyStoreCtx->fileStreamFactory,
                           &(keyStoreCtx->cryptoCore),
                           KEY_STORE_INSTANCE_NAME);

    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_init failed with error code %d!", __func__,
                        ret);
        return false;
    }

    return true;
}

void KeyStoreContext_dtor(KeyStoreContext* keyStoreCtx)
{
    ChanMuxClient_dtor(&(keyStoreCtx->chanMuxClient));
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM)));
    AesNvm_dtor(AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)));
    SeosSpiffs_dtor(&(keyStoreCtx->fs));
    FileStreamFactory_dtor(keyStoreCtx->fileStreamFactory);
    SeosKeyStore_deInit(&(keyStoreCtx->keyStore.parent));
    SeosCrypto_deInit(&(keyStoreCtx->cryptoCore.parent));
}

seos_err_t testKeyDataRetreival(SeosKeyStoreCtx* keyStoreCtx,
                                const char* keyName,
                                size_t expectedKeySize,
                                const char* expectedKeyData)
{
    seos_err_t err = SEOS_SUCCESS;
    SeosCryptoKey* readKey;
    // read the imported key
    err = SeosKeyStoreApi_getKey(keyStoreCtx, &readKey, keyName);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_getKey failed with error code %d!",
                        __func__, err);
        return err;
    }
    // check if the key data is correct
    if (memcmp(expectedKeyData, readKey->bytes, expectedKeySize) != 0
        || readKey->lenBits != expectedKeySize * 8
        || readKey->algorithm != SeosCryptoCipher_Algorithm_AES_CBC_ENC
        || readKey->flags != 1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER)
    {
        Debug_LOG_ERROR("%s: Read key data is not correct!", __func__);
        Debug_LOG_DEBUG("\n\n   Read bytes = ");
        for (size_t i = 0; i < readKey->lenBits / 8; i++)
        {
            printf("%02x ", readKey->bytes[i]);
        }
        printf("\n  Read length = %d\n", readKey->lenBits);
        printf("\n  Read algorithm = %d\n", readKey->algorithm);
        printf("\n  Read flags = %d\n", readKey->flags);

        Debug_LOG_DEBUG("\n\n   Expected bytes = ");
        for (size_t i = 0; i < expectedKeySize; i++)
        {
            printf("%02x ", expectedKeyData[i]);
        }
        printf("\n  Expected length = %d\n", expectedKeySize * 8);
        printf("\n  Expected algorithm = %d\n", SeosCryptoCipher_Algorithm_AES_CBC_ENC);
        printf("\n  Expected flags = %d\n", 1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER);
        return SEOS_ERROR_GENERIC;
    }

    return err;
}
