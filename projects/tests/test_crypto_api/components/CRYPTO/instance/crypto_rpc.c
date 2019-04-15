#include "seos/SeosCryptoRpc.h"
#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
crypto_rpc_get_instance(SeosCryptoRpc_Handle* instance)
{
    static SeosCryptoRpc the_one;
    seos_err_t retval = SeosCryptoRpc_ctor(&the_one, server_dataport);
    *instance = &the_one;

    Debug_LOG_DEBUG("%s: created rpc object %p", __func__, *instance);

    return retval;
}
