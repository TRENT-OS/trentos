#include <stdio.h>
#include "ProxyNVM.h"
#include "CryptoBlockDevice.h"
#include "spiffs.h"
#include "ChanMux/ChanMuxClient.h"
#include "camkes.h"

#define MEM_SIZE            1024*128
#define LOG_PAGE_SIZE       256

char test_in_buf[4096] = {0};

ProxyNVM testProxyNVM;
ChanMuxClient testChanMuxClient;
CryptoBlockDevice testCryptoBlockDevice;
static spiffs fs;

static uint8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static uint8_t spiffs_fds[32*4];
static uint8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];

static int32_t spiffsRead(uint32_t addr, uint32_t size, uint8_t *dst) {
    CryptoBlockDevice_read(&testCryptoBlockDevice, addr, (char*)dst, size);
    return SPIFFS_OK;
}

static int32_t spiffsWrite(uint32_t addr, uint32_t size, uint8_t *src) {
    CryptoBlockDevice_write(&testCryptoBlockDevice, addr, (char*)src, size);
    return SPIFFS_OK;
}

static int32_t spiffsErase(uint32_t addr, uint32_t size) {
    CryptoBlockDevice_erase(&testCryptoBlockDevice, addr, size);
    return SPIFFS_OK;
}

static void printSpiffsContent(){
  spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;
  uint32_t total, used;

  printf("\nSpiffs content:\n");

  SPIFFS_opendir(&fs, "/", &d);

  while ((pe = SPIFFS_readdir(&d, pe))) {
    printf("%s [%04x] size:%i\n", pe->name, pe->obj_id, pe->size);
  }

  SPIFFS_closedir(&d);

  printf("\nChecking the file system...\n");
  if(SPIFFS_info(&fs, &total, &used) < 0){
    printf("%s: SPIFFS_info error %i", __func__, SPIFFS_errno(&fs));
  }
  printf("\nSPIFFS total available bytes: %d\nSPIFFS used bytes: %d\n", total, used);
}

static void spiffsMount() {
    spiffs_config cfg;
    cfg.phys_size = 128*1024;           // use 128k of spi flash
    cfg.phys_addr = 0;                  // start spiffs at start of spi flash
    cfg.phys_erase_block = 8*4096;      // according to datasheet
    cfg.log_block_size = 8*4096;        // let us not complicate things
    cfg.log_page_size = LOG_PAGE_SIZE;  // as we said
        
    cfg.hal_read_f = spiffsRead;
    cfg.hal_write_f = spiffsWrite;
    cfg.hal_erase_f = spiffsErase;
        
    int res = SPIFFS_mount(&fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), 0);

    if(res != 0){
        printf("%s: First try unsuccessful! Unmounting...", __func__);
        SPIFFS_unmount(&fs);
        printf("%s: Formatting...", __func__);
        SPIFFS_format(&fs);
        printf("%s: Mounting...", __func__);
        res = SPIFFS_mount(&fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), 0);
    }

    res == 0 ? printf("%s: Mount successful!", __func__) : printf("%s: Mount failed with error: %i", __func__, res);
}

static int InitProxyNVM(){
    printf("\n\n");
    bool success = ChanMuxClient_ctor(&testChanMuxClient, 6, (void*)chanMuxDataPort);

    if(!success){
        printf("Failed to construct testChanMuxClient!\n");
        return -1;
    }

    success = ProxyNVM_ctor(&testProxyNVM, &testChanMuxClient);

    if(!success){
        printf("Failed to construct testProxyNVM!\n");
        return -1;
    }
    return 0;
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
  fd = SPIFFS_open(&fs, "numbers", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
  if (fd < 0) {
    printf("%s: SPIFFS_open error %i", __func__, SPIFFS_errno(&fs));
  }
  if (SPIFFS_write(&fs, fd, outNumbers, sizeof(outNumbers)) < 0) {
    printf("%s: SPIFFS_write error %i", __func__, SPIFFS_errno(&fs));
  }
  if (SPIFFS_close(&fs, fd) < 0) {
    printf("%s: SPIFFS_close error %i", __func__, SPIFFS_errno(&fs));

  }
  printSpiffsContent();


  printf("\nReading the file with first %d numbers...\n", 10);
  fd = SPIFFS_open(&fs, "numbers", SPIFFS_RDWR, 0);
  if (fd < 0) {
    printf("%s: SPIFFS_open error %i", __func__, SPIFFS_errno(&fs));
  }
  if (SPIFFS_read(&fs, fd, inMessage, 21) < 0) {
    printf("%s: SPIFFS_read error %i", __func__, SPIFFS_errno(&fs));
  }
  if (SPIFFS_close(&fs, fd) < 0) {
    printf("%s: SPIFFS_close error %i", __func__, SPIFFS_errno(&fs));
  }
  printf("\nRead numbers:\n%s\n", inMessage);

  printf("\n--------------------DONE-----------------------");
  printf("\n\n");
}

int run(){
  uint8_t ret = InitProxyNVM();
  if(ret < 0){
    printf("Error initializing ProxyNVM!");
    return 0;
  }

  CryptoBlockDevice_ctor(&testCryptoBlockDevice, ProxyNVM_TO_NVM(&testProxyNVM), ProxyNVM_write, ProxyNVM_read, ProxyNVM_erase);

  spiffsMount();
  runSpiffsTest();
    
  return 0;
}