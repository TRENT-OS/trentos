#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;

char buf[1074] = {0};

int run()
{
    //INITIALIZATION
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
    //INITIALIZATION


    //GET_SIZE TEST
    size_t ret_value = ProxyNVM_getSize(ProxyNVM_TO_NVM(&testProxyNVM));

    if(ret_value >= 0){
        printf("\nGet_size operation successful!\nSize of the memory: %d\n", ret_value);

    }
    else{
        printf("\nGet_size operation failed with error code: %d\n", ret_value);
    }
    //GET_SIZE TEST


    //WRITE TEST 
    ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)(0x12345678), (const char*)buf, sizeof(buf));

    if(ret_value >= 0){
        printf("\nWrite operation successful!\n%d bytes written\n", ret_value);

    }
    else{
        printf("\nWrite operation failed with error code: %d\n", ret_value);
    }
    //WRITE TEST


    for(int i = 0; i < sizeof(buf); i++){
        buf[i] = 0;
    }


    //READ TEST
    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)(0x12e85678), buf, sizeof(buf));

    if(ret_value >= 0){
        printf("\nRead operation successful!\n%d bytes written\nMemory content:\n", ret_value);

        for(int i = 0; i < ret_value; i++){
            printf(" %02x", buf[i]);
            if((i + 1) % 20 == 0)
                printf("\n");
        }
        printf("\n\n");
    }
    else{
        printf("\nRead operation failed with error code: %d\n", ret_value);
    }
    //READ TEST
    
    return 0;
}
