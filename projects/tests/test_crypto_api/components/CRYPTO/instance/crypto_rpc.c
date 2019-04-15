#include "seos/crypto_rpc.h"
#include "LibDebug/Debug.h"

#include <camkes.h>

seos_err_t
crypto_rpc_get_instance(crypto_rpc_handle_t* instance)
{
    static crypto_rpc_ctx_t the_one;
    seos_err_t retval = crypto_rpc_init(&the_one, server_dataport);
    *instance = &the_one;

    Debug_LOG_DEBUG("%s: created rpc object %p", __func__, *instance);

    return retval;
}
