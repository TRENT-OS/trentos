/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include <stdio.h>
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "camkes.h"

#include "initTest.h"
#include "fileStreamUnitTests.h"
#include "fileStreamDemoTest.h"

int run()
{
    FileStreamFactory* fsFactory;
    FSContext fsContext;
    seos_err_t err = SEOS_ERROR_GENERIC;

    /************************** Initialization ****************************/
    if (!initializeTest(&fsContext))
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }

    fsFactory = SpiffsFileStreamFactory_TO_FILE_STREAM_FACTORY(
                    SpiffsFileStreamFactory_getInstance(&(fsContext.fs)));
    if (fsFactory == NULL)
    {
        Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance!",
                        __func__);
        return false;
    }

    /************************** Tests ****************************/
    Debug_LOG_INFO("\n\n\n\n**************************** Starting 'Test_Spiffs_FileStream_test_case_01' ****************************\n");
    err = test_createFileStream(fsFactory);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("\n\nTest_Spiffs_FileStream_test_case_01 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTest_Spiffs_FileStream_test_case_01 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'Test_Spiffs_FileStream_test_case_02' ****************************\n");
    err = test_writeFileStream(fsFactory);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("\n\nTest_Spiffs_FileStream_test_case_02 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTest_Spiffs_FileStream_test_case_02 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'Test_Spiffs_FileStream_test_case_03' ****************************\n");
    err = test_readFileStream(fsFactory);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("\n\nTest_Spiffs_FileStream_test_case_03 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTest_Spiffs_FileStream_test_case_03 succeeded!\n\n\n\n");
    }

    Debug_LOG_INFO("\n**************************** Starting 'Test_Spiffs_FileStream_test_case_04' ****************************\n");
    err = spiffsFileStreamDemoTest_run(fsFactory);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("\n\nTest_Spiffs_FileStream_test_case_04 FAILED!\n\n\n\n");
    }
    else
    {
        Debug_LOG_INFO("\n\nTest_Spiffs_FileStream_test_case_04 succeeded!\n\n\n\n");
    }

    SpiffsFileStreamFactory_dtor();
    deinitializeTest(&fsContext);

    return 0;
}
