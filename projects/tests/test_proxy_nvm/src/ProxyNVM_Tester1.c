#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE                        (1024*128)
#define CHANMUX_NVM_CHANNEL             6

#define TEST_SMALL_SECTION_LEN          (MEM_SIZE / PAGE_SIZE) //arbitrary small chunk of data
#define TEST_WHOLE_MEM_LEN              MEM_SIZE
#define TEST_SIZE_OUT_OF_BOUNDS_LEN     MEM_SIZE
#define TEST_ADDR_OUT_OF_BOUNDS_LEN     MEM_SIZE

#define TEST_SMALL_SECTION_ADDR         (MEM_SIZE / PAGE_SIZE) //arbitrary memory address != 0
#define TEST_WHOLE_MEM_ADDR             0
#define TEST_SIZE_OUT_OF_BOUNDS_ADDR    (MEM_SIZE / 2)
#define TEST_ADDR_OUT_OF_BOUNDS_ADDR    (MEM_SIZE * 2)

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;

unsigned char out_buf[MEM_SIZE] = {0};
unsigned char in_buf[MEM_SIZE] = {0};


void RunTest(size_t address, size_t length, const char* testName)
{
    printf("\n\n");
    for (u_int i = 0; i < length; i++)
    {
        out_buf[i] = (char)i;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM),
                                      address, (const char*)out_buf, length);

    if (ret_value == length)
    {
        Debug_LOG_INFO("\nChannel %d: %s: Write succeded!", CHANMUX_NVM_CHANNEL, testName);
    }
    else
    {
        Debug_LOG_ERROR("\nChannel %d: %s: Write failed!\nTried to write %d bytes but written only %d bytes.",
                        CHANMUX_NVM_CHANNEL, testName, length, ret_value);
        return;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), address,
                              (char*)in_buf, length);
    if (ret_value == length)
    {
        Debug_LOG_INFO("\nChannel %d: %s: Read succeded!", CHANMUX_NVM_CHANNEL, testName);
    }
    else
    {
        Debug_LOG_ERROR("\nChannel %d: %s: Read failed!\nTried to read %d bytes but read only %d bytes.",
                        CHANMUX_NVM_CHANNEL, testName, length, ret_value);
        return;
    }

    for (int i = 0; i < length; i++)
    {
        if (out_buf[i] != in_buf[i])
        {
            Debug_LOG_ERROR("\nChannel %d: %s: Read values corrupted!\nOn position %d written %02x, but read %02x",
                            CHANMUX_NVM_CHANNEL, testName, i, out_buf[i], in_buf[i]);
            return;
        }
    }
    Debug_LOG_INFO("\nChannel %d: %s: Read values match the write values!", CHANMUX_NVM_CHANNEL, testName);
}

int InitProxyNVM(size_t chan)
{
    printf("\n\n");
    bool isSuccess = ChanMuxClient_ctor(&testChanMuxClient, chan, chanMuxDataPort);

    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed to construct testChanMuxClient!\n");
        return -1;
    }

    isSuccess = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient, (char*)chanMuxDataPort, PAGE_SIZE);

    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed to construct testProxyNVM!\n");
        return -1;
    }

    return 0;
}

int run()
{
    printf("\n\n");
    Debug_LOG_INFO("Initializing ProxyNVM with channel:%d", CHANMUX_NVM_CHANNEL);

    int ret_value = InitProxyNVM(CHANMUX_NVM_CHANNEL);

    if (ret_value < 0)
    {
        Debug_LOG_ERROR("%s(): channel %u: Error initializing ProxyNVM! Errno:%d",
                        __func__, CHANMUX_NVM_CHANNEL, ret_value);
        return 0;
    }

    RunTest(TEST_SMALL_SECTION_ADDR, TEST_SMALL_SECTION_LEN, "TEST SMALL SECTION");
    RunTest(TEST_WHOLE_MEM_ADDR, TEST_WHOLE_MEM_LEN, "TEST WHOLE MEMORY");
    RunTest(TEST_SIZE_OUT_OF_BOUNDS_ADDR, TEST_SIZE_OUT_OF_BOUNDS_LEN, "TEST SIZE OUT OF BOUNDS");
    RunTest(TEST_ADDR_OUT_OF_BOUNDS_ADDR, TEST_ADDR_OUT_OF_BOUNDS_LEN, "TEST ADDRESS OUT OF BOUNDS");

    return 0;
    
}
