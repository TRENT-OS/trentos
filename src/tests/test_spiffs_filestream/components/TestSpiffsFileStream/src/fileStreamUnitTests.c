/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "fileStreamUnitTests.h"

#define FILE_PATH           "File1"
#define FILE_PATH_TOO_LONG  "SomeTooLongFilePathStringSomeTooLongFilePathString"
#define FILE_PATH_EMPTY     ""

#define WRITE_FILE_PATH     "writeFile"
#define WRITE_STRING        "test string"
#define WRITE_STRING_LEN    strlen(WRITE_STRING)

#define READ_FILE_PATH      "readFile"
#define READ_STRING         "some example read string"
#define READ_STRING_LEN     25

seos_err_t test_createFileStream(FileStreamFactory* fsFactory)
{
    FileStream* stream;

    /***************************** Negative cases *******************************/
    stream = FileStreamFactory_create(fsFactory, FILE_PATH_TOO_LONG,
                                      FileStream_OpenMode_W);
    if (stream != NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create should have failed because of too long path!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH_EMPTY,
                                      FileStream_OpenMode_W);
    if (stream != NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create should have failed because of empty path!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, NULL, FileStream_OpenMode_W);
    if (stream != NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create should have failed because path == NULL!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_r);
    if (stream != NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create should have failed because open_mode = r and file does not exist!");
        return SEOS_ERROR_ABORTED;
    }

    /***************************** Positive cases *******************************/
    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_w);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_a);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_r);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_W);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_A);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    stream = FileStreamFactory_create(fsFactory, FILE_PATH, FileStream_OpenMode_R);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    FileStreamFactory_destroy(fsFactory, stream,
                              1 << FileStream_DeleteFlags_DELETE);

    return SEOS_SUCCESS;
}

seos_err_t test_writeFileStream(FileStreamFactory* fsFactory)
{
    FileStream* stream;
    seos_err_t err = SEOS_ERROR_GENERIC;

    stream = FileStreamFactory_create(fsFactory, WRITE_FILE_PATH,
                                      FileStream_OpenMode_W);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    /***************************** Negative cases *******************************/
    err = Stream_write(FileStream_TO_STREAM(stream), NULL, WRITE_STRING_LEN);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_write should have failed (wrote 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    err = Stream_write(FileStream_TO_STREAM(stream), WRITE_STRING, 0);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_write should have failed (wrote 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    err = Stream_write(FileStream_TO_STREAM(stream), NULL, 0);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_write should have failed (wrote 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    /***************************** Positive cases *******************************/
    err = Stream_write(FileStream_TO_STREAM(stream), WRITE_STRING,
                       WRITE_STRING_LEN);
    if (err != WRITE_STRING_LEN)
    {
        Debug_LOG_ERROR("%s: Stream_write failed, return value is %zu", __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    FileStreamFactory_destroy(fsFactory, stream,
                              1 << FileStream_DeleteFlags_DELETE);

    return SEOS_SUCCESS;
}

seos_err_t test_readFileStream(FileStreamFactory* fsFactory)
{
    FileStream* stream;
    seos_err_t err = SEOS_ERROR_GENERIC;
    char buffer[READ_STRING_LEN] = {0};

    stream = FileStreamFactory_create(fsFactory, READ_FILE_PATH,
                                      FileStream_OpenMode_W);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    err = Stream_write(FileStream_TO_STREAM(stream), READ_STRING, READ_STRING_LEN);
    if (err != READ_STRING_LEN)
    {
        Debug_LOG_ERROR("%s: Stream_write failed, return value is %zu", __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    FileStreamFactory_destroy(fsFactory, stream, 1 << FileStream_DeleteFlags_CLOSE);

    stream = FileStreamFactory_create(fsFactory, READ_FILE_PATH,
                                      FileStream_OpenMode_R);
    if (stream == NULL)
    {
        Debug_LOG_ERROR("FileStreamFactory_create failed!");
        return SEOS_ERROR_ABORTED;
    }

    /***************************** Negative cases *******************************/
    err = Stream_read(FileStream_TO_STREAM(stream), NULL, READ_STRING_LEN);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_read should have failed (read 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    err = Stream_read(FileStream_TO_STREAM(stream), buffer, 0);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_read should have failed (read 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    err = Stream_read(FileStream_TO_STREAM(stream), NULL, 0);
    if (err != 0)
    {
        Debug_LOG_ERROR("%s: Stream_read should have failed (read 0 bytes), but return value is %zu",
                        __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    /***************************** Positive cases *******************************/
    err = Stream_read(FileStream_TO_STREAM(stream), buffer, READ_STRING_LEN);
    if (err != READ_STRING_LEN)
    {
        Debug_LOG_ERROR("%s: Stream_read failed, return value is %zu", __func__, err);
        return SEOS_ERROR_ABORTED;
    }

    if (strncmp(buffer, READ_STRING, READ_STRING_LEN) != 0)
    {
        Debug_LOG_ERROR("%s: Read string incorrectly read!", __func__);
        return SEOS_ERROR_ABORTED;
    }

    FileStreamFactory_destroy(fsFactory, stream,
                              1 << FileStream_DeleteFlags_DELETE);

    return SEOS_SUCCESS;
}