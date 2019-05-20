#include <stdio.h>
#include "ProxyNVM.h"
#include "spiffs.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE                        1024*128

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
static spiffs fs;

int InitProxyNVM(){
    printf("\n\n");
    bool success = ChanMuxClient_ctor(&testChanMuxClient, 6, (void*)chanMuxDataPort);

    if(!success){
        Debug_LOG_ERROR("Failed to construct testChanMuxClient!\n");
        return -1;
    }

    success = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient);

    if(!success){
        Debug_LOG_ERROR("Failed to construct testProxyNVM!\n");
        return -1;
    }
    return 0;
}

int run()
{
    uint8_t ret = InitProxyNVM();
    if(ret < 0){
        Debug_LOG_ERROR("Error initializing ProxyNVM!");
        return 0;
    }
    
    return 0;
}
