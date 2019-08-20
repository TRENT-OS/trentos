/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
/* Includes ------------------------------------------------------------------*/
#include "testKeyStore.h"
#include "SeosCryptoApi.h"
#include "SeosKeyStoreApi.h"
#include "camkes.h"

/* Defines -------------------------------------------------------------------*/
#define NVM_PARTITION_SIZE      (1024*128)

#define AES_BLOCK_LEN           16

#define KEY_BYTES        "f131830db44c54742fc3f3265f0f1a0c"
#define KEY_NAME         "MasterKey"
#define KEY_SIZE         32

/* Private functions prototypes ----------------------------------------------*/
static seos_err_t
aesEncrypt(SeosCryptoCtx* cryptoCtx,
           SeosCrypto_KeyHandle keyHandle,
           const char* data,
           size_t inDataSize,
           void** outBuf,
           size_t* outDataSize);

static seos_err_t
aesDecrypt(SeosCryptoCtx* cryptoCtx,
           SeosCrypto_KeyHandle keyHandle,
           const void* data,
           size_t inDataSize,
           void** outBuf,
           size_t* outDataSize);

static seos_err_t testAesForKey(SeosCryptoCtx* cryptoCtx,
                                SeosCrypto_KeyHandle keyHandle);

/* Public functions -----------------------------------------------------------*/
bool testKeyStore(SeosKeyStoreCtx* keyStoreCtx, SeosCryptoCtx* cryptoCtx,
                  bool generateKey)
{
    SeosCryptoKey* writeKey;
    SeosCryptoKey* readKey;
    seos_err_t err = SEOS_ERROR_GENERIC;

    /***************************** Import/generate the key and test AES positive case *******************************/
    Debug_LOG_INFO("\n\nStarting 'TestKeyStore_testCase_01'\n");

    if (generateKey)
    {
        err = SeosKeyStoreApi_generateKey(keyStoreCtx,
                                          &writeKey,
                                          KEY_NAME,
                                          SeosCryptoCipher_Algorithm_AES_CBC_DEC,
                                          1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                          KEY_SIZE * 8);
        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_ERROR("%s: SeosKeyStoreApi_generateKey failed with error code %d!",
                            __func__, err);
            return 0;
        }
    }
    else
    {
        err = SeosKeyStoreApi_importKey(keyStoreCtx,
                                        &writeKey,
                                        KEY_NAME,
                                        KEY_BYTES,
                                        SeosCryptoCipher_Algorithm_AES_CBC_ENC,
                                        1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                        KEY_SIZE * 8);
        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_ERROR("%s: SeosKeyStoreApi_importKey failed with error code %d!",
                            __func__, err);
            return 0;
        }
    }

    err = testAesForKey(cryptoCtx, writeKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testAesForKey failed with error code %d", __func__, err);
        return 0;
    }

    Debug_LOG_INFO("\n\nTestKeyStore_testCase_01 passed!\n");

    /***************************** Close the key and test AES negative case *****************************************/
    Debug_LOG_INFO("\n\nStarting 'TestKeyStore_testCase_02'\n");

    err = SeosKeyStoreApi_closeKey(keyStoreCtx, writeKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_closeKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    err = testAesForKey(cryptoCtx, writeKey);
    if (err != SEOS_ERROR_ABORTED)
    {
        Debug_LOG_ERROR("%s: testAesForKey expected to fail because of the closed key but returned error code %d",
                        __func__, err);
        return 0;
    }

    Debug_LOG_INFO("\n\nTestKeyStore_testCase_02 passed!\n");

    /***************************** Get the key and test AES positive case *******************************************/
    Debug_LOG_INFO("\n\nStarting 'TestKeyStore_testCase_03'\n");

    err = SeosKeyStoreApi_getKey(keyStoreCtx,
                                 &readKey,
                                 KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_getKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    err = testAesForKey(cryptoCtx, readKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: testAesForKey failed with error code %d", __func__, err);
        return 0;
    }

    Debug_LOG_INFO("\n\nTestKeyStore_testCase_03 passed!\n");

    /***************************** Delete the key test AES/get negative case ****************************************/
    Debug_LOG_INFO("\n\nStarting 'TestKeyStore_testCase_04'\n");

    err = SeosKeyStoreApi_deleteKey(keyStoreCtx,
                                    readKey,
                                    KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStoreApi_deleteKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    err = testAesForKey(cryptoCtx, readKey);
    if (err != SEOS_ERROR_ABORTED)
    {
        Debug_LOG_ERROR("%s: testAesForKey expected to fail because of the closed key but returned error code %d",
                        __func__, err);
        return 0;
    }

    err = SeosKeyStoreApi_getKey(keyStoreCtx,
                                 &readKey,
                                 KEY_NAME);
    if (err != SEOS_ERROR_NOT_FOUND)
    {
        Debug_LOG_ERROR("%s: Expected to receive a SEOS_ERROR_NOT_FOUND after reading the deleted key, but received an err code: %d! Exiting test...",
                        __func__, err);
        return 0;
    }

    Debug_LOG_INFO("\n\nTestKeyStore_testCase_04 passed!\n");

    return 1;
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