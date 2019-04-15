#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>

#include "seos/crypto_client.h"

int run()
{
    crypto_client_t client;
    crypto_rpc_handle_t rpc_handle = NULL;

    void const* data;

    crypto_rpc_get_instance(&rpc_handle);
    Debug_LOG_DEBUG("%s: got rpc object %p from server", __func__, rpc_handle);

    crypto_client_init(&client, rpc_handle, crypto_client_dataport);
    crypto_client_get_random_data(&client, 0, &data, 0);

    return 0;
}
