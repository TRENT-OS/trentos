#include <stdio.h>
#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE              1024*128
#define NUM_OF_TEST_STREAMS   3

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
AesNvm testAesNvm;
SeosSpiffs fs;
SpiffsFileStreamFactory *streamFactory;

bool initializeTest(){
  if(!ChanMuxClient_ctor(&testChanMuxClient, 6, (void*)chanMuxDataPort)){
    Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
    return false;
  }

  if(!ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient)){
    Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
    return false;
  }

  if(!AesNvm_ctor(&testAesNvm, ProxyNVM_TO_NVM(&testProxyNVM))){
    Debug_LOG_ERROR("%s: Failed to initialize AesNvm!", __func__);
    return false;
  }

  if(!SeosSpiffs_ctor(&fs, AesNvm_TO_NVM(&testAesNvm), MEM_SIZE, 0)){
    Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
    return false;
  }

  if(SeosSpiffs_mount(&fs) != SEOS_SUCCESS){
    Debug_LOG_ERROR("%s: Failed to mount spiffs!", __func__);
    return false;
  }

  streamFactory = SpiffsFileStreamFactory_getInstance(&fs);
  if(streamFactory == NULL){
    Debug_LOG_ERROR("%s: Failed to get the SpiffsFileStreamFactory instance!", __func__);
    return false;
  }

  return true;
}

int run(){
  if(!initializeTest()){
    return 0;
  }

  char filePath[3] = {'f', '0', '\0'};
  char writeBuf[4] = {'c', '=', '0', '\0'};
  char readBuf[4] = {0};

  SpiffsFileStream* streams[NUM_OF_TEST_STREAMS];

  printf("\n--------------------------------------------------------------------------------------\n\n\n");

  for(int i = 0; i < NUM_OF_TEST_STREAMS; i++){
    writeBuf[2] = '0' + i + 1;
    filePath[1] = '0' + i + 1;

    streams[i] = SpiffsFileStreamFactory_create(filePath, FileStream_OpenMode_W);

    if(SpiffsFileStream_write(SpiffsFileStream_TO_STREAM(streams[i]), writeBuf, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }
    if(SpiffsFileStream_seek(SpiffsFileStream_TO_FILE_STREAM(streams[i]), 0, FileStream_SeekMode_Begin) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }
    if(SpiffsFileStream_read(SpiffsFileStream_TO_STREAM(streams[i]), readBuf, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }

    printf("\nRead from file %d: %s", i + 1, readBuf);
    printf("\n\n\n--------------------------------------------------------------------------------------\n\n\n");
  }

  for(int i = 0; i < NUM_OF_TEST_STREAMS; i++){
    SpiffsFileStream_close(SpiffsFileStream_TO_STREAM(streams[i]));
  }
    
  return 0;
}