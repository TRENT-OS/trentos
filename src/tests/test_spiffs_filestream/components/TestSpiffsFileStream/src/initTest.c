/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "initTest.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define NVM_PARTITION_SIZE    (1024*128)
#define CHANMUX_NVM_CHANNEL   6

bool initializeTest(FSContext* fsContext)
{
    if (!ChanMuxClient_ctor(&(fsContext->chanMuxClient), CHANMUX_NVM_CHANNEL,
                            (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
        return false;
    }

    if (!ProxyNVM_ctor(&(fsContext->proxyNVM), &(fsContext->chanMuxClient),
                       (char*)chanMuxDataPort,
                       PAGE_SIZE))
    {
        Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
        return false;
    }

    if (!AesNvm_ctor(&(fsContext->aesNvm), ProxyNVM_TO_NVM(&(fsContext->proxyNVM))))
    {
        Debug_LOG_ERROR("%s: Failed to initialize AesNvm!", __func__);
        return false;
    }

    if (!SeosSpiffs_ctor(&(fsContext->fs), AesNvm_TO_NVM(&(fsContext->aesNvm)),
                         NVM_PARTITION_SIZE, 0))
    {
        Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
        return false;
    }

    seos_err_t ret = SeosSpiffs_mount(&(fsContext->fs));
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: spiffs mount failed with error code %d!", __func__, ret);
        return false;
    }

    return true;
}

bool deinitializeTest(FSContext* fsContext)
{
    ChanMuxClient_dtor(&(fsContext->chanMuxClient));
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&(fsContext->proxyNVM)));
    AesNvm_dtor(AesNvm_TO_NVM(&(fsContext->aesNvm)));
    SeosSpiffs_dtor(&(fsContext->fs));

    return true;
}