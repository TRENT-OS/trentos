/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
#include <stdio.h>
#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define NVM_PARTITION_SIZE    (1024*128)
#define NUM_OF_TEST_STREAMS   3
#define CHANMUX_NVM_CHANNEL   6

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
AesNvm testAesNvm;
SeosSpiffs fs;
FileStreamFactory* streamFactory;

bool initializeTest()
{
    if (!ChanMuxClient_ctor(&testChanMuxClient, CHANMUX_NVM_CHANNEL,
                            (void*)chanMuxDataPort))
    {
        Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
        return false;
    }

    if (!ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient, (char*)chanMuxDataPort,
                       PAGE_SIZE))
    {
        Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
        return false;
    }

    if (!AesNvm_ctor(&testAesNvm, ProxyNVM_TO_NVM(&testProxyNVM)))
    {
        Debug_LOG_ERROR("%s: Failed to initialize AesNvm!", __func__);
        return false;
    }

    if (!SeosSpiffs_ctor(&fs, AesNvm_TO_NVM(&testAesNvm), NVM_PARTITION_SIZE, 0))
    {
        Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
        return false;
    }

    seos_err_t ret = SeosSpiffs_mount(&fs);
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: spiffs mount failed with error code %d!", __func__, ret);
        return false;
    }

    streamFactory = SpiffsFileStreamFactory_TO_FILE_STREAM_FACTORY(
                        SpiffsFileStreamFactory_getInstance(&fs));
    if (streamFactory == NULL)
    {
        Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance!",
                        __func__);
        return false;
    }

    return true;
}

bool destroyContext()
{
    ChanMuxClient_dtor(&testChanMuxClient);
    ProxyNVM_dtor(ProxyNVM_TO_NVM(&testProxyNVM));
    AesNvm_dtor(AesNvm_TO_NVM(&testAesNvm));
    SeosSpiffs_dtor(&fs);
    SpiffsFileStreamFactory_dtor();

    return true;
}

int run()
{
    if (!initializeTest())
    {
        Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
        return 0;
    }

    char filePath[3] = {'f', '0', '\0'};
    char writeBuf[8] = {'c', '=', '0', ',', 'b', '=', '0', '\0'};
    char readBuf[8] = {0};
    char getBuf[4] = {0};
    size_t retValue = 0;

    FileStream* streams[NUM_OF_TEST_STREAMS];

    Debug_LOG_DEBUG("\n--------------------------------------------------------------------------------------\n\n\n");

    // Performing the tests for NUM_OF_TEST_STREAMS filestreams:
    for (int i = 0; i < NUM_OF_TEST_STREAMS; i++)
    {
        // creating the dummy data and the file name according to the index
        writeBuf[2] = '0' + i + 1;
        writeBuf[6] = '0' + i + 1;
        filePath[1] = '0' + i + 1;

        size_t writeLength = strlen(writeBuf);

        // todo =>   think about reordering the tests from less complex to more complex functions
        //           and in a way we do not execute unnecessary tests (not executing a read from file)
        //           if the write failed

        //    1)  create a filestream with a path: "fX" (X is the number of the file)
        streams[i] = FileStreamFactory_create(streamFactory, filePath,
                                              FileStream_OpenMode_W);

        //    2)  write dummy data to it ("c=X")
        retValue = Stream_write(FileStream_TO_STREAM(streams[i]), writeBuf,
                                writeLength);
        if (retValue != writeLength)
        {
            Debug_LOG_ERROR("%s: Stream_write failed! Return value: %d", __func__,
                            retValue);
        }

        //    3)  position the pointer to the beginning of the file
        retValue = FileStream_seek(streams[i], 0, FileStream_SeekMode_Begin);
        if (retValue != 0)
        {
            Debug_LOG_ERROR("%s: FileStream_seek failed! Return value: %d", __func__,
                            retValue);
        }

        //    4)  check that the available space on the file and that it is
        //        equal to the size of the write buffer (the pointer is at the
        //        beginning of the file and it's size is sizeof(writeBuf))
        retValue = Stream_available(FileStream_TO_STREAM(streams[i]));
        if (retValue == sizeof(writeBuf) - 1)
        {
            Debug_LOG_DEBUG("\n\nAvailable space in file %d is equal to file size\n",
                            i + 1);
        }
        else
        {
            Debug_LOG_ERROR("%s: File %d, expected available space = %d, but available space is = %d",
                            __func__, i + 1, sizeof(writeBuf) - 1, retValue);
        }

        //    5)  reopen the file as read-only
        streams[i] = SpiffsFileStream_reOpen(streams[i], FileStream_OpenMode_r);

        //    6)  try to write to read-only file and verify this will result in an error
        //        currently we only have SEOS_ERROR_GENERIC => todo: expand error messages
        retValue = Stream_write(FileStream_TO_STREAM(streams[i]), writeBuf,
                                writeLength);
        seos_err_t err = FileStream_error(streams[i]);
        if (retValue == 0 && err == SEOS_ERROR_GENERIC)
        {
            Debug_LOG_DEBUG("\n\nFile %d, unsuccesful write to read-only file!\n", i + 1);
        }
        else
        {
            Debug_LOG_ERROR("\n\nFile %d, write to read-only file succeded! Return value: %d, file error: %d\n",
                            i + 1, retValue, err);
        }

        //    7)  position the pointer to the beginning of the file
        retValue = FileStream_seek(streams[i], 0, FileStream_SeekMode_Begin);
        if (retValue != 0)
        {
            Debug_LOG_ERROR("%s: FileStream_seek failed! Return value: %d", __func__,
                            retValue);
        }

        //    8)  read the entire written data
        retValue = Stream_read(FileStream_TO_STREAM(streams[i]), readBuf,
                               writeLength);
        if (retValue != writeLength)
        {
            Debug_LOG_ERROR("%s: Stream_read failed! Return value: %d", __func__, retValue);
        }

        //    9)  position the pointer to the beginning of the file
        retValue = FileStream_seek(streams[i], 0, FileStream_SeekMode_Begin);
        if (retValue != 0)
        {
            Debug_LOG_ERROR("%s: FileStream_seek failed! Return value: %d", __func__,
                            retValue);
        }

        //    10) read just the first part of the file (up to the first delimiter)
        retValue = Stream_get(FileStream_TO_STREAM(streams[i]), getBuf, sizeof(getBuf),
                              ",", 0);
        if (retValue < 0)
        {
            Debug_LOG_ERROR("%s: Stream_get failed! Return value: %d", __func__, retValue);
        }

        //    11) check that there are no errors on the file
        err = FileStream_error(streams[i]);
        if (err == SEOS_SUCCESS)
        {
            Debug_LOG_DEBUG("\n\nFile %d has no errors!\n", i + 1);
        }
        else
        {
            Debug_LOG_DEBUG("\n\nError %d on file %d!\n", err, i + 1);
        }

        Debug_LOG_DEBUG("\n\nRead from file %d: %s\n", i + 1, readBuf);
        Debug_LOG_DEBUG("\n\nGet from file %d: %s\n", i + 1, getBuf);
        Debug_LOG_DEBUG("\n\n\n--------------------------------------------------------------------------------------\n\n\n");
    }

    // Closing all of the opened files
    for (int i = 0; i < NUM_OF_TEST_STREAMS; i++)
    {
        FileStreamFactory_destroy(streamFactory, streams[i], 1 << FileStream_DeleteFlags_CLOSE);
    }

    destroyContext();

    Debug_LOG_DEBUG("\nSuccesfully destroyed the context.\n");

    return 0;
}