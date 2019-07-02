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
#define NUM_OF_TEST_STREAMS   3
#define CHANMUX_NVM_CHANNEL   6
#define MASTER_KEY            "f131830db44c54742fc3f3265f0f1a0c"
#define KEY_LENGTH_IN_BYTES   32

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

    SeosCryptoKey key;
    SeosCryptoKey readKey;
    char keyBytes[33] = {0};

    seos_err_t err = SeosCryptoKey_ctor(&key,
                             NULL,
                             SeosCryptoCipher_Algorithm_AES_CBC_ENC,
                             1 << SeosCryptoKey_Flags_IS_ALGO_CIPHER,
                             MASTER_KEY,
                             KEY_LENGTH_IN_BYTES * 8);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosCryptoKey_ctor failed to construct the key with error code %d!",
                        __func__, err);
        return 0;
    }

    err = SeosKeyStore_importKey(&keyStore, "MasterKey", &key);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_importKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    err = SeosKeyStore_getKey(&keyStore, "MasterKey", &readKey, keyBytes);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: SeosKeyStore_importKey failed with error code %d!",
                        __func__, err);
        return 0;
    }

    printf("\nReadkey data\n    bytes => %s\n    lenBits => %d\n", readKey.bytes, readKey.lenBits);

    return 0;
}