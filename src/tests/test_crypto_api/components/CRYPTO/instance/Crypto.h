/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#pragma once

#include "SeosCryptoRpc.h"
#include "SeosKeyStoreRpc.h"
#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
Crypto_getRpcHandle(SeosCryptoRpc_Handle* instance);

void
Crypto_closeRpcHandle(SeosCryptoRpc_Handle instance);

seos_err_t
KeyStore_getRpcHandle(SeosKeyStoreRpc_Handle* instance);

void
KeyStore_closeRpcHandle(SeosKeyStoreRpc_Handle instance);
