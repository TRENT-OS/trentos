/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include "ChanMux/ChanMuxClient.h"
#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "SeosKeyStore.h"
#include "SeosKeyStoreClient.h"
#include "SeosKeyStoreRpc.h"
#include "SeosKeyStoreApi.h"

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

bool keyStoreContext_ctor(KeyStoreContext*  keyStoreCtx,
                          uint8_t           channelNum,
                          void*             dataport);

bool keyStoreContext_dtor(KeyStoreContext* keyStoreCtx);

seos_err_t
KeyStore_getRpcHandle(SeosKeyStoreRpc_Handle* instance);

void
KeyStore_closeRpcHandle(SeosKeyStoreRpc_Handle instance);

