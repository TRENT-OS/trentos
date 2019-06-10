#include <stdio.h>
#include "ProxyNVM.h"
#include "AesNvm.h"
#include "SeosSpiffs.h"
#include "SpiffsFileStream.h"
#include "SpiffsFileStreamFactory.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE            1024*128

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

  return true;
}

int run(){
  if(!initializeTest()){
    return 0;
  }

  char buffer[4] = {0};
  char name[3] = {'f', '0', '\0'};
  char writeBuf[4] = {'c', '=', '0', '\0'};

  for(int i = 0; i < 5; i++){
    writeBuf[2] = '0' + i;
    name[1] = '0' + i;

    printf("\n\n");
    SpiffsFileStream *spiffsStream = SpiffsFileStreamFactory_create(name, FileStream_OpenMode_W);
    if(SpiffsFileStream_write(SpiffsFileStream_TO_STREAM(spiffsStream), writeBuf, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }
    if(SpiffsFileStream_seek(SpiffsFileStream_TO_FILE_STREAM(spiffsStream), 0, FileStream_SeekMode_Begin) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }
    if(SpiffsFileStream_read(SpiffsFileStream_TO_STREAM(spiffsStream), buffer, strlen(writeBuf)) < 0){
      Debug_LOG_ERROR("%s: SpiffsFileStream_write failed!", __func__);
    }
    SpiffsFileStream_close(SpiffsFileStream_TO_STREAM(spiffsStream));
    printf("\nRead from file: %s\n", buffer);
  }
    
  return 0;
}