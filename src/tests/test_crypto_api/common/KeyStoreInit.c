/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
/* Includes ------------------------------------------------------------------*/
#include "KeyStoreInit.h"

/* Defines -------------------------------------------------------------------*/
#define NVM_PARTITION_SIZE      (1024*128)

/* Public functions -----------------------------------------------------------*/
bool keyStoreContext_ctor(KeyStoreContext*  keyStoreCtx,
                          uint8_t           channelNum,
                          void*             dataport)
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
    return true;
}

bool keyStoreContext_dtor(KeyStoreContext* keyStoreCtx)
{
    ChanMuxClient_dtor(&(keyStoreCtx->chanMuxClient));
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&(keyStoreCtx->proxyNVM)));
    AesNvm_dtor(AesNvm_TO_NVM(&(keyStoreCtx->aesNvm)));
    SeosSpiffs_dtor(&(keyStoreCtx->fs));
    FileStreamFactory_dtor(keyStoreCtx->fileStreamFactory);

    return true;
}