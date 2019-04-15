#pragma once

#include "seos/SeosCryptoRpc.h"
#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
Crypto_getRpcHandle(SeosCryptoRpc_Handle* instance);

void
Crypto_closeRpcHandle(SeosCryptoRpc_Handle instance);
