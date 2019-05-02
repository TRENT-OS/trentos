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

    for (int i = 0; i < 10; i++)
    {
        SeosCryptoClient_getRandomData(&client, 0, &data, 16);
        Debug_PRINTF("Printing random bytes...");
        for (unsigned j = 0; j < 16; j++)
        {
            Debug_PRINTF(" 0x%02x", ((char*) data)[j]);
        }
        Debug_PRINTF("\n");
    }

    return 0;
}
