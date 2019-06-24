#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE                        1024*128        

#define TEST_SMALL_SECTION_LEN          (MEM_SIZE / PAGE_SIZE) //arbitrary small chunk of data
#define TEST_WHOLE_MEM_LEN              MEM_SIZE
#define TEST_SIZE_OUT_OF_BOUNDS_LEN     MEM_SIZE
#define TEST_ADDR_OUT_OF_BOUNDS_LEN     MEM_SIZE

#define TEST_SMALL_SECTION_ADDR         (MEM_SIZE / PAGE_SIZE) //arbitrary memory address != 0
#define TEST_WHOLE_MEM_ADDR             0
#define TEST_SIZE_OUT_OF_BOUNDS_ADDR    (MEM_SIZE / 2)
#define TEST_ADDR_OUT_OF_BOUNDS_ADDR    (MEM_SIZE * 2)

ProxyNVM testProxyNVM, testProxyNVM2;
ChanMuxClient testChanMuxClient, testChanMuxClient2;

unsigned char out_buf[MEM_SIZE] = {0};
unsigned char in_buf[MEM_SIZE] = {0};
uint8_t chan1 = 6;
uint8_t chan2 = 7;

void RunTest(size_t address, size_t length, const char* testName){
    printf("\n\n");
    for(int i = 0; i < length; i++){
        out_buf[i] = i%256;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)address, (const char*)out_buf, (size_t)length);
    if(ret_value == length){
        Debug_LOG_INFO("\n%s: Write succeded!", testName);
    }
    else{
        Debug_LOG_ERROR("\n%s: Write failed!\nTried to write %d bytes but written only %d bytes.", testName, length, ret_value);
        return;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), (size_t)address, (char*)in_buf, (size_t)length);
    if(ret_value == length){
        Debug_LOG_INFO("\n%s: Read succeded!", testName);
    }
    else{
        Debug_LOG_ERROR("\n%s: Read failed!\nTried to read %d bytes but read only %d bytes.", testName, length, ret_value);
        return;
    }

    for(int i = 0; i < length; i++){
        if(out_buf[i] != in_buf[i]){
            Debug_LOG_ERROR("\n%s: Read values corrupted!\nOn position %d written %02x, but read %02x", testName, i, out_buf[i], in_buf[i]);
            return;
        }
    }
    Debug_LOG_INFO("\n%s: Read values match the write values!", testName);
}

int InitProxyNVM(uint chan){
    printf("\n\n");
    if (chan == 6){
    bool success = ChanMuxClient_ctor(&testChanMuxClient, chan, (void*)chanMuxDataPort);
    
        if(!success){
            Debug_LOG_ERROR("Failed to construct testChanMuxClient!\n");
            return -1;
        }

        success = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient);

        if(!success){
            Debug_LOG_ERROR("Failed to construct testProxyNVM!\n");
            return -1;
        }
    }
    else if (chan == 7){
        bool success = ChanMuxClient_ctor(&testChanMuxClient2, chan, (void*)chanMuxDataPort);
    
        if(!success){
            Debug_LOG_ERROR("Failed to construct testChanMuxClient!\n");
            return -1;
        }

        success = ProxyNVM_ctor(&testProxyNVM2, &testChanMuxClient2);

        if(!success){
            Debug_LOG_ERROR("Failed to construct testProxyNVM!\n");
            return -1;
        }
    }

    return 0;
}

int run()
{
    printf("\n\n");
    Debug_LOG_INFO("Initializing ProxyNVM! Make sure to initialize the corresponding counterpart.");
    Debug_LOG_INFO("Initializing ProxyNVM with channel:%d", chan1);
    uint8_t ret = InitProxyNVM(chan1);

    Debug_LOG_INFO("Initializing ProxyNVM with channel:%d", chan2);
    ret = InitProxyNVM(chan2);
    printf("\n\n");

    if(ret < 0){
        Debug_LOG_ERROR("Error initializing ProxyNVM!");
        return 0;
    }

    RunTest(TEST_SMALL_SECTION_ADDR, TEST_SMALL_SECTION_LEN, "TEST SMALL SECTION");
    RunTest(TEST_WHOLE_MEM_ADDR, TEST_WHOLE_MEM_LEN, "TEST WHOLE MEMORY");
    RunTest(TEST_SIZE_OUT_OF_BOUNDS_ADDR, TEST_SIZE_OUT_OF_BOUNDS_LEN, "TEST SIZE OUT OF BOUNDS");
    RunTest(TEST_ADDR_OUT_OF_BOUNDS_ADDR, TEST_ADDR_OUT_OF_BOUNDS_LEN, "TEST ADDRESS OUT OF BOUNDS");
    
    return 0;
}
