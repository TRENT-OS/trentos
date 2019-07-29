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
                            uint8_t         channelNum,
                            void*           dataport);

bool keyStoreContext_dtor(KeyStoreContext* keyStoreCtx);

bool testKeyStoreLocally(SeosKeyStore keyStore);

