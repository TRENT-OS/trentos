#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE                    1024*128        

#define TEST_SMALL_SECTION_LEN      50
#define TEST_WHOLE_MEM_LEN          MEM_SIZE
#define TEST_OUT_OF_BOUNDS_LEN      MEM_SIZE

#define TEST_SMALL_SECTION_ADDR     100
#define TEST_WHOLE_MEM_ADDR         0
#define TEST_OUT_OF_BOUNDS_ADDR     (MEM_SIZE / 2)

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;

char out_buf[MEM_SIZE] = {0};
char in_buf[MEM_SIZE] = {0};

void Run_firstTest(){
    printf("\n\n");
    for(int i = 0; i < TEST_SMALL_SECTION_LEN; i++){
        out_buf[i] = i%127;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_SMALL_SECTION_ADDR, (const char*)out_buf, (size_t)TEST_SMALL_SECTION_LEN);
    if(ret_value == TEST_SMALL_SECTION_LEN){
        Debug_LOG_INFO("\nTEST_SMALL_SECTION: Write succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_SMALL_SECTION: Write failed!\nTried to write %d bytes but written only %d bytes.", TEST_SMALL_SECTION_LEN, ret_value);
        return;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_SMALL_SECTION_ADDR, in_buf, (size_t)TEST_SMALL_SECTION_LEN);
    if(ret_value == TEST_SMALL_SECTION_LEN){
        Debug_LOG_INFO("\nTEST_SMALL_SECTION: Read succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_SMALL_SECTION: Read failed!\nTried to read %d bytes but read only %d bytes.", TEST_SMALL_SECTION_LEN, ret_value);
        return;
    }

    for(int i = 0; i < TEST_SMALL_SECTION_LEN; i++){
        if(out_buf[i] != in_buf[i]){
            Debug_LOG_ERROR("\nTEST_SMALL_SECTION: Read values corrupted!\nOn position %d written %02x, but read %02x", i, out_buf[i], in_buf[i]);
            return;
        }
    }
    Debug_LOG_INFO("\nTEST_SMALL_SECTION: Read values match the write values!");
}

void Run_secondTest(){
    printf("\n\n");
    for(int i = 0; i < TEST_WHOLE_MEM_LEN; i++){
        out_buf[i] = i%127;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_WHOLE_MEM_ADDR, (const char*)out_buf, (size_t)TEST_WHOLE_MEM_LEN);
    if(ret_value == TEST_WHOLE_MEM_LEN){
        Debug_LOG_INFO("\nTEST_WHOLE_MEM: Write succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_WHOLE_MEM: Write failed!\nTried to write %d bytes but written only %d bytes.", TEST_WHOLE_MEM_LEN, ret_value);
        return;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_WHOLE_MEM_ADDR, in_buf, (size_t)TEST_WHOLE_MEM_LEN);
    if(ret_value == TEST_WHOLE_MEM_LEN){
        Debug_LOG_INFO("\nTEST_WHOLE_MEM: Read succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_WHOLE_MEM: Read failed!\nTried to read %d bytes but read only %d bytes.", TEST_WHOLE_MEM_LEN, ret_value);
        return;
    }

    for(int i = 0; i < TEST_WHOLE_MEM_LEN; i++){
        if(out_buf[i] != in_buf[i]){
            Debug_LOG_ERROR("\nTEST_WHOLE_MEM: Read values corrupted!\nOn position %d written %02x, but read %02x", i, out_buf[i], in_buf[i]);
            return;
        }
    }
    Debug_LOG_INFO("\nTEST_WHOLE_MEM: Read values match the write values!");
}

void Run_thirdTest(){
    printf("\n\n");
    for(int i = 0; i < TEST_OUT_OF_BOUNDS_LEN; i++){
        out_buf[i] = i%127;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_OUT_OF_BOUNDS_ADDR, (const char*)out_buf, (size_t)TEST_OUT_OF_BOUNDS_LEN);
    if(ret_value == TEST_OUT_OF_BOUNDS_LEN){
        Debug_LOG_INFO("\nTEST_OUT_OF_BOUNDS: Write succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_OUT_OF_BOUNDS: Write failed!\nTried to write %d bytes but written only %d bytes.", TEST_OUT_OF_BOUNDS_LEN, ret_value);
        return;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)TEST_OUT_OF_BOUNDS_ADDR, in_buf, (size_t)TEST_OUT_OF_BOUNDS_LEN);
    if(ret_value == TEST_OUT_OF_BOUNDS_LEN){
        Debug_LOG_INFO("\nTEST_OUT_OF_BOUNDS: Read succeded!");
    }
    else{
        Debug_LOG_ERROR("\nTEST_OUT_OF_BOUNDS: Read failed!\nTried to read %d bytes but read only %d bytes.", TEST_OUT_OF_BOUNDS_LEN, ret_value);
        return;
    }

    for(int i = 0; i < TEST_OUT_OF_BOUNDS_LEN; i++){
        if(out_buf[i] != in_buf[i]){
            Debug_LOG_ERROR("\nTEST_OUT_OF_BOUNDS: Read values corrupted!\nOn position %d written %02x, but read %02x", i, out_buf[i], in_buf[i]);
            return;
        }
    }
    Debug_LOG_INFO("\nTEST_OUT_OF_BOUNDS: Read values match the write values!");
}

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

    Run_firstTest();
    Run_secondTest();
    Run_thirdTest();
    
    return 0;
}
