#include <stdio.h>
#include "ProxyNVM.h"
#include "CryptoBlockDevice.h"
#include "SeosSpiffs.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE            1024*128

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
CryptoBlockDevice testCryptoBlockDevice;
SeosSpiffs fs;

static bool InitProxyNVM(){
    bool success = ChanMuxClient_ctor(&testChanMuxClient, 6, (void*)chanMuxDataPort);

    if(!success){
        Debug_LOG_ERROR("%s: Failed to construct testChanMuxClient!", __func__);
        return false;
    }

    success = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient);

    if(!success){
        Debug_LOG_ERROR("%s: Failed to construct testProxyNVM!", __func__);
        return false;
    }
    return true;
}

static void printSpiffsContent(){
  spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;
  uint32_t total, used;

  printf("\nSpiffs content:\n");

  SPIFFS_opendir(&fs.spif_fs, "/", &d);

  while ((pe = SPIFFS_readdir(&d, pe))) {
    printf("%s [%04x] size:%i\n", pe->name, pe->obj_id, pe->size);
  }

  SPIFFS_closedir(&d);

  printf("\nChecking the file system...\n");
  if(SPIFFS_info(&fs.spif_fs, &total, &used) < 0){
    Debug_LOG_ERROR("%s: SPIFFS_info error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  printf("\nSPIFFS total available bytes: %d\nSPIFFS used bytes: %d\n", total, used);
}

static void runSpiffsTest(){
  uint8_t outNumbers[21] = {0};
  uint8_t inMessage[21] = {0};
  spiffs_file fd;

  for(int i = 0, j = 0; i < 20; i+=2){
    outNumbers[i] = j + '0';
    outNumbers[i + 1] = ' ';
    j++;
  }
  outNumbers[20] = '\0';

  printSpiffsContent();

  printf("\nCreating a file with first %d numbers...\n", 10);
  fd = SPIFFS_open(&fs.spif_fs, "numbers", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
  if (fd < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_open error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  if (SPIFFS_write(&fs.spif_fs, fd, outNumbers, sizeof(outNumbers)) < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_write error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  if (SPIFFS_close(&fs.spif_fs, fd) < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_close error %i", __func__, SPIFFS_errno(&fs.spif_fs));

  }
  printSpiffsContent();


  printf("\nReading the file with first %d numbers...\n", 10);
  fd = SPIFFS_open(&fs.spif_fs, "numbers", SPIFFS_RDWR, 0);
  if (fd < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_open error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  if (SPIFFS_read(&fs.spif_fs, fd, inMessage, 21) < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_read error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  if (SPIFFS_close(&fs.spif_fs, fd) < 0) {
    Debug_LOG_ERROR("%s: SPIFFS_close error %i", __func__, SPIFFS_errno(&fs.spif_fs));
  }
  printf("\nRead numbers:\n%s\n", inMessage);

  printf("\n--------------------DONE-----------------------");
}

int run(){
  if(!InitProxyNVM()){
    Debug_LOG_ERROR("%s: Failed to initialize ProxyNVM!", __func__);
    return 0;
  }

  if(!CryptoBlockDevice_ctor(&testCryptoBlockDevice, ProxyNVM_TO_NVM(&testProxyNVM))){
    Debug_LOG_ERROR("%s: Failed to initialize CryptoBlockDevice!", __func__);
    return 0;
  }

  if(!SeosSpiffs_ctor(&fs, CryptoBlockDevice_TO_NVM(&testCryptoBlockDevice), MEM_SIZE, 0)){
    Debug_LOG_ERROR("%s: Failed to initialize spiffs!", __func__);
    return 0;
  }

  if(!SeosSpiffs_mount(&fs)){
    Debug_LOG_ERROR("%s: Failed to mount spiffs!", __func__);
    return 0;
  }

  runSpiffsTest();
    
  return 0;
}