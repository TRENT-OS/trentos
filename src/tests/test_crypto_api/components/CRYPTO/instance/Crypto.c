#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
Crypto_getRpcHandle(SeosCryptoRpc_Handle* instance)
{
    static SeosCrypto    cryptoCore;
    static SeosCryptoRpc the_one;

    seos_err_t retval = SeosCrypto_init(&cryptoCore,
                                        malloc, free, NULL, NULL);
    if (SEOS_SUCCESS == retval)
    {
        retval = SeosCryptoRpc_init(&the_one, &cryptoCore, serverDataport);
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
