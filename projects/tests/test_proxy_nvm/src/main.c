#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;

char buf[1074*2] = {0};

int run()
{
    printf("Starting main...\n");

    for(int i = 0; i < sizeof(buf); i++){
        buf[i] = i%127;
    }

    bool success = ChanMuxClient_ctor(&testChanMuxClient, 6, (void*)chanMuxDataPort);

    if(!success){
        printf("Failed to construct testChanMuxClient!\n");
        return 0;
    }

    success = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient);

    if(!success){
        printf("Failed to construct testProxyNVM!\n");
        return 0;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)(0x12345678), (const char*)buf, sizeof(buf));

    if(ret_value == 0){
        printf("\nWrite operation successful!\n");

    }
    else
    {
        printf("\nWrite operation failed with error code: %d\n", ret_value);
    }
    
    
    return 0;
}
