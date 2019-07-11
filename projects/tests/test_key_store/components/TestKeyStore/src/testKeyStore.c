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
#include "SeosCryptoDigest.h"
#include "SeosCryptoCipher.h"
#include "camkes.h"

/* Defines -------------------------------------------------------------------*/
#define NVM_PARTITION_SIZE      (1024*128)
#define CHANMUX_NVM_CHANNEL_1   6
#define CHANMUX_NVM_CHANNEL_2   7

#define MASTER_KEY_BYTES        "f131830db44c54742fc3f3265f0f1a0cf131830db44c54742fc3f3265f0f1a0c"
#define MASTER_KEY_NAME         "MasterKey"
#define MASTER_KEY_SIZE         64

#define GENERATED_KEY_NAME      "GeneratedKey"
#define GENERATED_KEY_SIZE      256

/* Private types ---------------------------------------------------------*/
typedef struct KeyStoreContext
{
    ProxyNVM proxyNVM;
    ChanMuxClient chanMuxClient;
    AesNvm aesNvm;
    SeosSpiffs fs;
    FileStreamFactory* fileStreamFactory;
    SeosKeyStore keyStore;
}KeyStoreContext;

/* Private functions prototypes ----------------------------------------------*/
bool KeyStoreContext_ctor(KeyStoreContext* keyStoreCtx, uint8_t channelNum);
bool KeyStoreContext_dtor(KeyStoreContext* keyStoreCtx);

/* Private variables ---------------------------------------------------------*/

/* Test ----------------------------------------------------------------------*/
int run()
{
    KeyStoreContext keyStoreCtx_1;
    //KeyStoreContext keyStoreCtx_2;

    if (!KeyStoreContext_ctor(&keyStoreCtx_1, CHANMUX_NVM_CHANNEL_1))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }

    SeosCryptoKey writeKey;
    SeosCryptoKey readKey;
    SeosCryptoKey generatedKey;
    char keyBytes[MASTER_KEY_SIZE + 1] = {0};
    char newKeyWriteBytes[GENERATED_KEY_SIZE + 1] = {0};
    char newKeyReadBytes[GENERATED_KEY_SIZE + 1] = {0};
    size_t keysize = 0;

    SeosKeyStore_KeyType keyType;

    SeosKeyStore_KeyTypeCtor(&keyType,
                                NULL,
                                SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                                1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                MASTER_KEY_SIZE * 8);

    // create the key that is to be imported to the keyStore
    seos_err_t err = SeosCryptoKey_ctor(&writeKey,
                                        NULL,
                                        SeosCryptoCipher_Algorithm_AES_CBC_ENC,
                                        1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                                        MASTER_KEY_BYTES,
                                        MASTER_KEY_SIZE * 8);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoKey_ctor failed to construct the key with error code %d!",
                        __func__, err);
        return 0;
    }

    // import the created key
    err = SeosKeyStore_importKey(&(keyStoreCtx_1.keyStore), MASTER_KEY_NAME, &writeKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_importKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe key is succesfully imported!\n");


    // get the size of the imported key (to be used before the getKey to allocate memory
    // if the user does not know the size of the key)
    err = SeosKeyStore_getKeySizeBytes(&(keyStoreCtx_1.keyStore), MASTER_KEY_NAME, &keysize);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_getKeySizeBytes failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // check if the size returnd by getSize is correct
    if (keysize != MASTER_KEY_SIZE)
    {
        Debug_LOG_ERROR("%s: Expected keySize is %d, but the read keySize is %d! Exiting test...",
                        __func__, MASTER_KEY_SIZE, keysize);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe keySize is succesfully read!\n");

    // read the imported key
    err = SeosKeyStore_getKey(&(keyStoreCtx_1.keyStore), MASTER_KEY_NAME, &readKey, keyBytes, &keyType);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_getKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // check if the key data is correct
    if (strcmp(MASTER_KEY_BYTES, readKey.bytes) != 0
        || readKey.lenBits != MASTER_KEY_SIZE * 8)
    {
        Debug_LOG_ERROR("%s: Read key data is not correct!\n    Key bytes => %s\n    Key lenBits => %d\nExiting test...",
                        __func__, readKey.bytes, readKey.lenBits);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe key data is succesfully read!\n");

    // delete the key
    err = SeosKeyStore_deleteKey(&(keyStoreCtx_1.keyStore), MASTER_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_deleteKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    // check if the key is actaully deleted by verifying that the getKey results in an error
    err = SeosKeyStore_getKey(&(keyStoreCtx_1.keyStore), MASTER_KEY_NAME, &readKey, keyBytes, &keyType);
    if (err != SEOS_ERROR_NOT_FOUND)
    {
        Debug_LOG_ERROR("%s: Expected to receive a SEOS_ERROR_NOT_FOUND after reading the deleted key, but received an err code: %d! Exiting test...",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe key is succesfully deleted!\n");

    // generate new key
    err = SeosKeyStore_generateKey(&(keyStoreCtx_1.keyStore), &generatedKey, GENERATED_KEY_NAME, newKeyWriteBytes, &keyType);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_generateKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe generated key:\n   Bytes = ");
    for (size_t i = 0; i < generatedKey.lenBits / 8; i++)
    {
        printf("%02x ", generatedKey.bytes[i]);
    }
    printf("\n  Length = %d\n", generatedKey.lenBits);

    // read the generated key
    err = SeosKeyStore_getKey(&(keyStoreCtx_1.keyStore), GENERATED_KEY_NAME, &readKey, newKeyReadBytes, &keyType);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_getKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    // check if the key data is correct
    Debug_LOG_DEBUG("\n\nThe read key:\n   Bytes = ");
    for (size_t i = 0; i < readKey.lenBits / 8; i++)
    {
        printf("%02x ", readKey.bytes[i]);
    }
    printf("\n  Length = %d\n", readKey.lenBits);

    KeyStoreContext_dtor(&keyStoreCtx_1);

    return 0;
}

/* Private functions ---------------------------------------------------------*/
bool KeyStoreContext_ctor(KeyStoreContext* keyStoreCtx, uint8_t channelNum)
{
    if (!ChanMuxClient_ctor(&(keyStoreCtx->chanMuxClient), channelNum,
                            (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to construct chanMuxClient, channel %d!", __func__, channelNum);
        return false;
    }

    if (!ProxyNVM_ctor(&(keyStoreCtx->proxyNVM), &(keyStoreCtx->chanMuxClient), (char*)chanMuxDataPort,
                       PAGE_SIZE))
    {
        Debug_LOG_ERROR("%s: Failed to construct proxyNVM, channel %d!", __func__, channelNum);
        return false;
    }

    if (!AesNvm_ctor(&(keyStoreCtx->aesNvm), ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM))))
    {
        Debug_LOG_ERROR("%s: Failed to initialize AesNvm, channel %d!", __func__, channelNum);
        return false;
    }

    if (!SeosSpiffs_ctor(&(keyStoreCtx->fs), AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)), NVM_PARTITION_SIZE, 0))
    {
        Debug_LOG_ERROR("%s: Failed to initialize spiffs, channel %d!", __func__, channelNum);
        return false;
    }

    seos_err_t ret = SeosSpiffs_mount(&(keyStoreCtx->fs));
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: spiffs mount failed with error code %d, channel %d!", __func__, ret, channelNum);
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

    if (!SeosKeyStore_ctor(&(keyStoreCtx->keyStore), keyStoreCtx->fileStreamFactory))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the key store, channel %d!", __func__, channelNum);
        return false;
    }

    return true;
}

bool KeyStoreContext_dtor(KeyStoreContext* keyStoreCtx)
{
    ChanMuxClient_dtor(&(keyStoreCtx->chanMuxClient));
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM)));
    AesNvm_dtor(AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)));
    SeosSpiffs_dtor(&(keyStoreCtx->fs));
    FileStreamFactory_dtor(keyStoreCtx->fileStreamFactory);
    SeosKeyStore_dtor(&(keyStoreCtx->keyStore));

    return true;
}