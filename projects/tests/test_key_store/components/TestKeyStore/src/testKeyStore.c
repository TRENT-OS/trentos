/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
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

#define NVM_PARTITION_SIZE    (1024*128)
#define CHANMUX_NVM_CHANNEL   6
#define MASTER_KEY_BYTES      "f131830db44c54742fc3f3265f0f1a0cf131830db44c54742fc3f3265f0f1a0c"
#define MASTER_KEY_NAME       "MasterKey"
#define MASTER_KEY_SIZE       64

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
AesNvm testAesNvm;
SeosSpiffs fs;
FileStreamFactory* streamFactory;
SeosKeyStore keyStore;

bool initializeTest()
{
    if (!ChanMuxClient_ctor(&testChanMuxClient, CHANMUX_NVM_CHANNEL,
                            (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
        return false;
    }

    if (!ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient, (char*)chanMuxDataPort,
                       PAGE_SIZE))
    {
        Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
        return false;
    }

    if (!AesNvm_ctor(&testAesNvm, ProxyNVM_TO_NVM(&testProxyNVM)))
    {
        Debug_LOG_ERROR("%s: Failed to initialize AesNvm!", __func__);
        return false;
    }

    if (!SeosSpiffs_ctor(&fs, AesNvm_TO_NVM(&testAesNvm), NVM_PARTITION_SIZE, 0))
    {
        Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
        return false;
    }

    seos_err_t ret = SeosSpiffs_mount(&fs);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: spiffs mount failed with error code %d!", __func__, ret);
        return false;
    }

    streamFactory = SpiffsFileStreamFactory_TO_FILE_STREAM_FACTORY(
                        SpiffsFileStreamFactory_getInstance(&fs));
    if (streamFactory == NULL)
    {
        Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance!",
                        __func__);
        return false;
    }

    if (!SeosKeyStore_ctor(&keyStore, streamFactory))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the key store!", __func__);
        return false;
    }

    return true;
}

bool destroyContext()
{
    ChanMuxClient_dtor(&testChanMuxClient);
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&testProxyNVM));
    AesNvm_dtor(AesNvm_TO_NVM(&testAesNvm));
    SeosSpiffs_dtor(&fs);
    FileStreamFactory_dtor(streamFactory);
    SeosKeyStore_dtor(&keyStore);

    return true;
}

int run()
{
    if (!initializeTest())
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }

    SeosCryptoKey writeKey;
    SeosCryptoKey readKey;
    char keyBytes[MASTER_KEY_SIZE + 1] = {0};
    size_t keysize = 0;

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
    err = SeosKeyStore_importKey(&keyStore, MASTER_KEY_NAME, &writeKey);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_importKey failed with error code %d!",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe key is succesfully imported!\n");


    // get the size of the imported key (to be used before the getKey to allocate memory
    // if the user does not know the size of the key)
    err = SeosKeyStore_getKeySizeBytes(&keyStore, MASTER_KEY_NAME, &keysize);
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
    err = SeosKeyStore_getKey(&keyStore, MASTER_KEY_NAME, &readKey, keyBytes);
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
    err = SeosKeyStore_deleteKey(&keyStore, MASTER_KEY_NAME);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_deleteKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    // check if the key is actaully deleted by verifying that the getKey results in an error
    err = SeosKeyStore_getKey(&keyStore, MASTER_KEY_NAME, &readKey, keyBytes);
    if (err != SEOS_ERROR_NOT_FOUND)
    {
        Debug_LOG_ERROR("%s: Expected to receive a SEOS_ERROR_NOT_FOUND after reading the deleted key, but received an err code: %d! Exiting test...",
                        __func__, err);
        return 0;
    }
    Debug_LOG_DEBUG("\n\nThe key is succesfully deleted!\n");

    return 0;
}