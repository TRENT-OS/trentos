#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
Crypto_getRpcHandle(SeosCryptoRpc_Handle* instance)
{
    static SeosCryptoRpc the_one;
    seos_err_t retval = SeosCryptoRpc_ctor(&the_one, serverDataport);
    *instance = &the_one;

    Debug_LOG_DEBUG("%s: created rpc object %p", __func__, *instance);

    return retval;
}

void
Crypto_closeRpcHandle(SeosCryptoRpc_Handle instance)
{
    /// TODO
}
