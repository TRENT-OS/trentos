/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "fileStreamUnitTests.h"

#define FILE_PATH                       "File1"
#define EXAMPLE_STRING                  "c = 1, b = 2"
#define EXAMPLE_STRING_FIRST_PART       "c = 1"
#define EXAMPLE_STRING_LEN              13
#define EXAMPLE_STRING_FIRST_PART_LEN   6

seos_err_t spiffsFileStreamDemoTest_run(FileStreamFactory* fsFactory)
{
    char readBuf[EXAMPLE_STRING_LEN] = {0};
    char getBuf[EXAMPLE_STRING_LEN] = {0};
    size_t retValue = 0;
    seos_err_t err = SEOS_ERROR_GENERIC;

    FileStream* stream;

    // 1)  create a filestream with a path: "FILE_PATH"
    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_W);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    // 2)  write EXAMPLE_STRING to it
    retValue = Stream_write(FileStream_TO_STREAM(stream), EXAMPLE_STRING,
                            EXAMPLE_STRING_LEN);
    err = FileStream_error(stream);
    if (retValue != EXAMPLE_STRING_LEN || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: Stream_write failed! Return value: %d, err = %zu",
                        __func__, retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 3)  position the pointer to the beginning of the file
    retValue = FileStream_seek(stream, 0, FileStream_SeekMode_Begin);
    err = FileStream_error(stream);
    if (retValue != 0 || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: Stream_write failed! Return value: %d, err = %zu",
                        __func__, retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 4)  check that the available space on the file and that it is
    //     equal to the size of the write buffer (the pointer is at the
    //     beginning of the file and it's size is sizeof(writeBuf))
    retValue = Stream_available(FileStream_TO_STREAM(stream));
    err = FileStream_error(stream);
    if (retValue != EXAMPLE_STRING_LEN || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: Stream_available failed! Return value: %d, err = %zu",
                        __func__, retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 5)  reopen the file as read-only
    stream = SpiffsFileStream_reOpen(stream, FileStream_OpenMode_r);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    // 6)  read the entire written data
    retValue = Stream_read(FileStream_TO_STREAM(stream), readBuf,
                           EXAMPLE_STRING_LEN);
    err = FileStream_error(stream);
    if (retValue != EXAMPLE_STRING_LEN || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: Stream_read failed! Return value: %d, err = %zu", __func__,
                        retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 7)  verify that the read string matches the EXAMPLE_STRING
    if (strncmp(readBuf, EXAMPLE_STRING, EXAMPLE_STRING_LEN) != 0)
    {
        Debug_LOG_ERROR("The string read from the file (%s) does not match the written string (%s)",
                        readBuf, EXAMPLE_STRING);
    }

    // 8)  position the pointer to the beginning of the file
    retValue = FileStream_seek(stream, 0, FileStream_SeekMode_Begin);
    err = FileStream_error(stream);
    if (retValue != 0 || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: FileStream_seek failed! Return value: %d, err = %zu",
                        __func__, retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 9) read just the first part of the file (up to the first delimiter)
    retValue = Stream_get(FileStream_TO_STREAM(stream), getBuf, sizeof(getBuf), ",",
                          0);
    err = FileStream_error(stream);
    if (retValue < 0 || err != SEOS_SUCCESS)
    {
        Debug_LOG_ERROR("%s: Stream_get failed! Return value: %d, err = %zu", __func__,
                        retValue, err);
        return SEOS_ERROR_ABORTED;
    }

    // 10)  verify that the read string matches the EXAMPLE_STRING_FIRST_PART
    if (strncmp(getBuf, EXAMPLE_STRING_FIRST_PART,
                EXAMPLE_STRING_FIRST_PART_LEN) != 0)
    {
        Debug_LOG_ERROR("The string read from the file (%s) does not match the written string (%s)",
                        getBuf, EXAMPLE_STRING_FIRST_PART);
        return SEOS_ERROR_ABORTED;
    }

    // 11) Delete the opened file
    FileStreamFactory_destroy(fsFactory, stream,
                              1 << FileStream_DeleteFlags_DELETE);

    return SEOS_SUCCESS;
}
