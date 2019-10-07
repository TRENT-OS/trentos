/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"

typedef struct FSContext
{
    ProxyNVM proxyNVM;
    ChanMuxClient chanMuxClient;
    AesNvm aesNvm;
    SeosSpiffs fs;
} FSContext;


bool initializeTest(FSContext* fsContext);
bool deinitializeTest(FSContext* fsContext);