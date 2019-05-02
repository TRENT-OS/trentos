#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;

const char buf[4] = {0x0a, 0x0b, 0x0c, 0x0d};
char input_buf[4] = {0};

int run()
{
    printf("Starting main...\n");

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

    size_t written_bytes = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)(0x12345678), buf, sizeof(buf));
    size_t read_bytes = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)(0x12345678), input_buf, sizeof(input_buf));

    printf("Written bytes: %d\nRead bytes: %d\n", written_bytes, read_bytes);
    printf("Read memory:");

    for(int i = 0; i < read_bytes; i++){
        printf("\n  %d", input_buf[i]);
    }

    return 0;
}
