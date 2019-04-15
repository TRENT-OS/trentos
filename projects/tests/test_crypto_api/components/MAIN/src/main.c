#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>

#include "seos/SeosCryptoClient.h"

int run()
{
    SeosCryptoClient client;
    SeosCryptoRpc_Handle rpc_handle = NULL;

    void const* data;

    crypto_rpc_get_instance(&rpc_handle);
    Debug_LOG_DEBUG("%s: got rpc object %p from server", __func__, rpc_handle);

    SeosCryptoClient_ctor(&client, rpc_handle, crypto_client_dataport);
    SeosCryptoClient_getRandomData(&client, 0, &data, 0);

    return 0;
}
