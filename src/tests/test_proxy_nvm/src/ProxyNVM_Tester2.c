#include <stdio.h>
#include "LibDebug/Debug.h"
#include "camkes.h"
#include "ProxyNVM_common.h"
#include "ChanMux_config.h"


#define MEM_SIZE                        (1024*128)

#define TEST_SMALL_SECTION_LEN          (MEM_SIZE / PAGE_SIZE) //arbitrary small chunk of data
#define TEST_WHOLE_MEM_LEN              MEM_SIZE
#define TEST_SIZE_OUT_OF_BOUNDS_LEN     MEM_SIZE
#define TEST_ADDR_OUT_OF_BOUNDS_LEN     MEM_SIZE

#define TEST_SMALL_SECTION_ADDR         (MEM_SIZE / PAGE_SIZE) //arbitrary memory address != 0
#define TEST_WHOLE_MEM_ADDR             0
#define TEST_SIZE_OUT_OF_BOUNDS_ADDR    (MEM_SIZE / 2)
#define TEST_ADDR_OUT_OF_BOUNDS_ADDR    (MEM_SIZE * 2)



int run()
{
    int ret_value = ProxyNVMTest_init(CHANNEL_NVM_USER2_DATA);

    if (ret_value < 0)
    {
        Debug_LOG_ERROR("%s(): channel %u: Error initializing ProxyNVM! Errno:%d",
                        __func__, CHANNEL_NVM_USER2_DATA, ret_value);
        return -1;
    }

    bool isSuccess;

    isSuccess = ProxyNVMTest_run(TEST_SMALL_SECTION_ADDR, TEST_SMALL_SECTION_LEN,
                                 "TEST SMALL SECTION");
    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed TEST SMALL SECTION!\n");
    }

    isSuccess = ProxyNVMTest_run(TEST_WHOLE_MEM_ADDR, TEST_WHOLE_MEM_LEN,
                                 "TEST WHOLE MEMORY");
    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed TEST WHOLE MEMORY!\n");
    }

    isSuccess = ProxyNVMTest_run(TEST_SIZE_OUT_OF_BOUNDS_ADDR,
                                 TEST_SIZE_OUT_OF_BOUNDS_LEN,
                                 "TEST SIZE OUT OF BOUNDS");
    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed TEST SIZE OUT OF BOUNDS!\n");
    }

    isSuccess = ProxyNVMTest_run(TEST_ADDR_OUT_OF_BOUNDS_ADDR,
                                 TEST_ADDR_OUT_OF_BOUNDS_LEN,
                                 "TEST ADDRESS OUT OF BOUNDS");
    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed TEST ADDRESS OUT OF BOUNDS!\n");
    }

    return 0;

}
