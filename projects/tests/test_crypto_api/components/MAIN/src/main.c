#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>

#include "seos/SeosCryptoClient.h"

int run()
{
    SeosCryptoClient client;
    SeosCryptoRpc_Handle rpcHandle = NULL;

    void const* data;

    Crypto_getRpcHandle(&rpcHandle);
    Debug_LOG_DEBUG("%s: got rpc object %p from server", __func__, rpcHandle);

    SeosCryptoClient_ctor(&client, rpcHandle, cryptoClientDataport);
    SeosCryptoClient_getRandomData(&client, 0, &data, 0);

    return 0;
}
