/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "ProxyNVM.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"


/* Defines -------------------------------------------------------------------*/
#define MEM_SIZE                        (1024*128)

/* Instance variables ---------------------------------------------------------*/

static ProxyNVM testProxyNVM;
static ChanMuxClient testChanMuxClient;

static unsigned char out_buf[MEM_SIZE] = {0};
static unsigned char in_buf[MEM_SIZE] = {0};

/* Public functions ----------------------------------------------------------*/

int ProxyNVMTest_init(unsigned int chan)
{

    bool isSuccess = ChanMuxClient_ctor(&testChanMuxClient, chan, chanMuxDataPort);

    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed to construct testChanMuxClient!\n");
        return -1;
    }

    isSuccess = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient,
                              (char*)chanMuxDataPort, PAGE_SIZE);

    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed to construct testProxyNVM!\n");
        return -1;
    }

    return 0;
}

bool ProxyNVMTest_run(size_t address, size_t length, const char* testName)
{

    for (u_int i = 0; i < length; i++)
    {
        out_buf[i] = (char)i;
    }

    size_t ret_value = ProxyNVM_write(ProxyNVM_TO_NVM(&testProxyNVM),
                                      address, (const char*)out_buf, length);

    if (ret_value == length)
    {
        Debug_LOG_INFO("\nChannel %d: %s: Write succeded!", testProxyNVM.chanmux->chan,
                       testName);
    }
    else
    {
        Debug_LOG_ERROR("\nChannel %d: %s: Write failed!\nTried to write %d bytes but written only %d bytes.",
                        testProxyNVM.chanmux->chan, testName, length, ret_value);
        return false;
    }

    ret_value = ProxyNVM_read(ProxyNVM_TO_NVM(&testProxyNVM), address,
                              (char*)in_buf, length);
    if (ret_value == length)
    {
        Debug_LOG_INFO("\nChannel %d: %s: Read succeded!", testProxyNVM.chanmux->chan,
                       testName);
    }
    else
    {
        Debug_LOG_ERROR("\nChannel %d: %s: Read failed!\nTried to read %d bytes but read only %d bytes.",
                        testProxyNVM.chanmux->chan, testName, length, ret_value);
        return false;
    }

    for (int i = 0; i < length; i++)
    {
        if (out_buf[i] != in_buf[i])
        {
            Debug_LOG_ERROR("\nChannel %d: %s: Read values corrupted!\nOn position %d written %02x, but read %02x",
                            testProxyNVM.chanmux->chan, testName, i, out_buf[i], in_buf[i]);
            return false;
        }
    }
    Debug_LOG_INFO("\nChannel %d: %s: Read values match the write values!",
                   testProxyNVM.chanmux->chan, testName);

    return true;
}
