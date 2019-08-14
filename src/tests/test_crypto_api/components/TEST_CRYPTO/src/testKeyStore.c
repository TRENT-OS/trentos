/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
/* Includes ------------------------------------------------------------------*/
#include "testKeyStore.h"
#include "camkes.h"

/* Defines -------------------------------------------------------------------*/
#define NVM_PARTITION_SIZE      (1024*128)

#define AES_BLOCK_LEN           16

#define MASTER_KEY_BYTES        "f131830db44c54742fc3f3265f0f1a0c"
#define MASTER_KEY_NAME         "MasterKey"
#define MASTER_KEY_SIZE         32

#define GENERATED_KEY_NAME      "GeneratedKey"
#define GENERATED_KEY_SIZE      32

/* Private functions prototypes ----------------------------------------------*/
static seos_err_t
aesEncrypt(SeosCryptoCtx* cryptoCtx, SeosCrypto_KeyHandle keyHandle,
           const char* data, size_t inDataSize, void** outBuf, size_t* outDataSize);
static seos_err_t
aesDecrypt(SeosCryptoCtx* cryptoCtx, SeosCrypto_KeyHandle keyHandle,
           const void* data, size_t inDataSize, void** outBuf, size_t* outDataSize);

static seos_err_t testAesForKey(SeosCryptoCtx* cryptoCtx,
                                SeosCrypto_KeyHandle keyHandle);

/* Private variables ---------------------------------------------------------*/

/* Public functions -----------------------------------------------------------*/
bool testKeyStore(SeosKeyStoreApi* keyStoreApi, SeosCryptoCtx* cryptoCtx)
{
    SeosCryptoKey* masterKey;
    SeosCryptoKey* generatedKey;
    SeosCryptoKey* readKey;

    /***************************** TEST KEY IMPORT *******************************/
    seos_err_t err = SeosKeyStoreApi_importKey(keyStoreApi,
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

    // get the imported key
    err = SeosKeyStoreApi_getKey(keyStoreApi,
                                 &readKey,
                                 MASTER_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_getKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // use the imported key for aes encryption
    testAesForKey(cryptoCtx, readKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testAesForKey failed for the imported key with error code %d",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nAES encryption/decryption succesfully performed with the imported key!\n");

    // delete the imported key
    err = SeosKeyStoreApi_deleteKey(keyStoreApi, masterKey,
                                    MASTER_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_deleteKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    // check if the key is actaully deleted by verifying that the getKey results in an error
    err = SeosKeyStoreApi_getKey(keyStoreApi,
                                 &readKey,
                                 MASTER_KEY_NAME);
    if (err != SEOS_ERROR_NOT_FOUND)
    {
        Debug_LOG_ERROR("%s: Expected to receive a SEOS_ERROR_NOT_FOUND after reading the deleted key, but received an err code: %d! Exiting test...",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe master key is succesfully deleted!\n");

    /***************************** TEST KEY GENERATION *******************************/
    err = SeosKeyStoreApi_generateKey(keyStoreApi,
                                      &generatedKey,
                                      GENERATED_KEY_NAME,
                                      SeosCryptoCipher_Algorithm_AES_CBC_DEC,
                                      1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                      GENERATED_KEY_SIZE * 8);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_generateKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // get the generated key
    err = SeosKeyStoreApi_getKey(keyStoreApi,
                                 &readKey,
                                 GENERATED_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_getKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // use the imported key for aes encryption
    testAesForKey(cryptoCtx, readKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testAesForKey failed for the generated key with error code %d",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nAES encryption/decryption succesfully performed with the generated key!\n");

    return 0;
}

bool keyStoreContext_ctor(KeyStoreContext*  keyStoreCtx,
                          uint8_t         channelNum,
                          void*           dataport)
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

    return true;
}

bool keyStoreContext_dtor(KeyStoreContext* keyStoreCtx)
{
    ChanMuxClient_dtor(&(keyStoreCtx->chanMuxClient));
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM)));
    AesNvm_dtor(AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)));
    SeosSpiffs_dtor(&(keyStoreCtx->fs));
    FileStreamFactory_dtor(keyStoreCtx->fileStreamFactory);
    SeosCrypto_deInit(&(keyStoreCtx->cryptoCore.parent));

    return true;
}

/* Private functions ---------------------------------------------------------*/
static seos_err_t
aesEncrypt(SeosCryptoCtx* cryptoCtx, SeosCrypto_KeyHandle keyHandle,
           const char* data, size_t inDataSize, void** outBuf, size_t* outDataSize)
{
    seos_err_t err = SEOS_ERROR_GENERIC;
    SeosCrypto_CipherHandle handle;

    *outDataSize = AES_BLOCK_LEN;

    err = SeosCryptoApi_cipherInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                                   keyHandle,
                                   NULL, 0);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherInit failed with error code %d",
                        __func__, err);
        return err;
    }

    err = SeosCryptoApi_cipherUpdate(cryptoCtx,
                                     handle,
                                     data,
                                     inDataSize,
                                     outBuf,
                                     outDataSize);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherUpdate failed with error code %d",
                        __func__, err);
    }

    err = SeosCryptoApi_cipherClose(cryptoCtx, handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherClose failed with error code %d",
                        __func__, err);
    }

    return err;
}

static seos_err_t
aesDecrypt(SeosCryptoCtx* cryptoCtx, SeosCrypto_KeyHandle keyHandle,
           const void* data, size_t inDataSize, void** outBuf, size_t* outDataSize)
{
    seos_err_t err = SEOS_ERROR_GENERIC;
    SeosCrypto_CipherHandle handle;

    *outDataSize = AES_BLOCK_LEN;

    err = SeosCryptoApi_cipherInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoCipher_Algorithm_AES_EBC_DEC,
                                   keyHandle,
                                   NULL, 0);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherInit failed with error code %d",
                        __func__, err);
        return err;
    }

    err = SeosCryptoApi_cipherUpdate(cryptoCtx,
                                     handle,
                                     data,
                                     inDataSize,
                                     outBuf,
                                     outDataSize);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherUpdate failed with error code %d",
                        __func__, err);
    }

    err = SeosCryptoApi_cipherClose(cryptoCtx, handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoApi_cipherClose failed with error code %d",
                        __func__, err);
    }

    return err;
}

static seos_err_t testAesForKey(SeosCryptoCtx* cryptoCtx,
                                SeosCrypto_KeyHandle keyHandle)
{
    char buffEnc[AES_BLOCK_LEN] = {0};
    char buffDec[AES_BLOCK_LEN] = {0};

    void* outputEncrypt = &buffEnc;
    void* outputDecrypt = &buffDec;

    size_t decOutSize = 0;
    size_t encOutSize = 0;

    // use the imported key for aes encryption
    seos_err_t err = aesEncrypt(cryptoCtx, keyHandle, "0123456789ABCDEF",
                                strlen("0123456789ABCDEF"), &outputEncrypt, &decOutSize);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: aesEncrypt failed with error code %d",
                        __func__, err);
        return err;
    }
    // use the imported key for aes decryption of the original string
    err = aesDecrypt(cryptoCtx, keyHandle, outputEncrypt, decOutSize,
                     &outputDecrypt, &encOutSize);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: aesDecrypt failed with error code %d",
                        __func__, err);
        return err;
    }
    // check if the decrypted string is the same as the original string
    if (strncmp("0123456789ABCDEF", ((char*)outputDecrypt), AES_BLOCK_LEN) != 0)
    {
        Debug_LOG_ERROR("%s: AES encryption/decryption failed! Decrypted block: %s, original block: %s",
                        __func__, ((char*)outputDecrypt), "0123456789ABCDEF");
        return SEOS_ERROR_GENERIC;
    }

    return err;
}