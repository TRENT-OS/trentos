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
FileStreamFactory *streamFactory;

bool initializeTest(){
  if(!ChanMuxClient_ctor(&testChanMuxClient, CHANMUX_NVM_CHANNEL, (void*)chanMuxDataPort)){
    Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
    return false;
  }

  if(!ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient, (char*)chanMuxDataPort, PAGE_SIZE)){
    Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
    return false;
  }

  if(!AesNvm_ctor(&testAesNvm, ProxyNVM_TO_NVM(&testProxyNVM))){
    Debug_LOG_ERROR("%s: Failed to initialize AesNvm!", __func__);
    return false;
  }

  if(!SeosSpiffs_ctor(&fs, AesNvm_TO_NVM(&testAesNvm), NVM_PARTITION_SIZE, 0)){
    Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
    return false;
  }

  seos_err_t ret = SeosSpiffs_mount(&fs);
  if(ret != SEOS_SUCCESS){
    Debug_LOG_ERROR("%s: spiffs mount failed with error code %d!", __func__, ret);
    return false;
  }

  streamFactory = SpiffsFileStreamFactory_TO_FILE_STREAM_FACTORY(SpiffsFileStreamFactory_getInstance(&fs));
  if(streamFactory == NULL){
    Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance!", __func__);
    return false;
  }

  return true;
}

bool destroyContext(){
  ChanMuxClient_dtor(&testChanMuxClient);
  ProxyNVM_dtor(ProxyNVM_TO_NVM(&testProxyNVM));
  AesNvm_dtor(AesNvm_TO_NVM(&testAesNvm));
  SeosSpiffs_dtor(&fs);
  SpiffsFileStreamFactory_dtor();

  return true;
}

int run(){
  if(!initializeTest()){
    Debug_LOG_ERROR("%s: Failed to initialize the test!", __func__);
    return 0;
  }

  char filePath[3] = {'f', '0', '\0'};
  char writeBuf[4] = {'c', '=', '0', '\0'};
  char readBuf[4] = {0};

  FileStream* streams[NUM_OF_TEST_STREAMS];

  Debug_LOG_DEBUG("\n--------------------------------------------------------------------------------------\n\n\n");

  // Performing the following test for NUM_OF_TEST_STREAMS filestreams:
  //    1) create a filestream with a path: "fX" (X is the number of the file)
  //    2) write dummy data to it ("c=X")
  //    3) position the ponter to the begining of the file
  //    4) read the written data
  for(int i = 0; i < NUM_OF_TEST_STREAMS; i++){
    writeBuf[2] = '0' + i + 1;
    filePath[1] = '0' + i + 1;

    streams[i] = FileStreamFactory_create(streamFactory, filePath, FileStream_OpenMode_W);

    if(Stream_write(FileStream_TO_STREAM(streams[i]), writeBuf, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: FileStream_write failed!", __func__);
    }
    if(FileStream_seek(streams[i], 0, FileStream_SeekMode_Begin) < 0){
      Debug_LOG_ERROR("%s: FileStream_write failed!", __func__);
    }
    if(Stream_read(FileStream_TO_STREAM(streams[i]), readBuf, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: FileStream_write failed!", __func__);
    }

    Debug_LOG_DEBUG("\nRead from file %d: %s", i + 1, readBuf);
    Debug_LOG_DEBUG("\n\n\n--------------------------------------------------------------------------------------\n\n\n");
  }

  // Closing all of the opened files
  for(int i = 0; i < NUM_OF_TEST_STREAMS; i++){
    FileStreamFactory_destroy(streamFactory, streams[i]);
  }

  destroyContext();

  Debug_LOG_DEBUG("\nSuccesfully destroyed the context.\n");
    
  return 0;
}