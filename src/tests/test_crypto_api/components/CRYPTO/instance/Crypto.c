/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include "LibDebug/Debug.h"

#include "Crypto.h"
#include "SeosCrypto.h"
#include <string.h>
#include <camkes.h>

static SeosCrypto    cryptoCore;

int entropyFunc(void*           ctx,
                unsigned char*  buf,
                size_t          len)
{
    // This would be the platform specific function to obtain entropy
    memset(buf, 0, len);
    return 0;
}

seos_err_t
Crypto_getRpcHandle(SeosCryptoRpc_Handle* instance)
{
    static SeosCryptoRpc the_one;

    seos_err_t retval = SeosCrypto_init(&cryptoCore, malloc, free, entropyFunc,
                                        NULL);
    if (SEOS_SUCCESS == retval)
    {
        retval = SeosCryptoRpc_init(&the_one, &cryptoCore, cryptoServerDataport);
        *instance = &the_one;

        if (SEOS_SUCCESS == retval)
        {
            Debug_LOG_TRACE("%s: created rpc object %p", __func__, *instance);
        }
    }
    return retval;
}

void
Crypto_closeRpcHandle(SeosCryptoRpc_Handle instance)
{
    /// TODO
}

