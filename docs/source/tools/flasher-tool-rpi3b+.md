# Flasher Tool for RPi3B+

## General

The Flasher Tool is a TRENTOS system, which has to be built first and
afterwards can be deployed on the target.

It can be used to dump a previously created RamDisk image to the SPI
flash of a Raspberry Pi 3 Model B+ (RPi3B+) system.

Refer to the [RamDisk Generator Tool](ram-disk-generator-tool.md) on how to
create a RamDisk image.

## Configuration

Inside the project folder of the **`rpi3_flasher`** tool, there is a
default **`flash.c`** provided suitable for the IoT App demo on the
RPi3B+.

Any other **`flash.c`** file can be created with the **`rdgen`** tool
and then used to build a new **`rpi3_flasher`** image.

Provide your **`flash.c`** to the **`rpi3_flasher`** project folder
before building the image.

```shell
# get in to the sdk package folder
cd <sdk_root_directory>

# copy the flash.c file
cp <path_to_custom_flash_c_parent_dir>/flash.c sdk/tools/rpi3_flasher/flash.c
```

## Build

Build the **`rpi3_flasher`** image with the following command:

```shell
# trigger the build
sdk/scripts/open_trentos_build_env.sh \
    sdk/build-system.sh \
    sdk/tools/rpi3_flasher \
    rpi3 \
    build-rpi3-Debug-flasher \
    -DCMAKE_BUILD_TYPE=Debug
```

The resulting image can be found at
**`build-rpi3-Debug-flasher/images/os_image.elf`**.

## Run

**Info:** The RPi3B+ requires SD cards to use an MBR partition table. If this is
not the case (e.g. partition table uses GPT format) the board will not boot at
all. Make sure to verify that the card used for the RPi3B+ is MBR ("MS-DOS")
formatted before continuing with the next instructions. More information on SD
card partitioning can be found in the
[Using an SD Card with a TRENTOS System](../platform-support/using-sd-card-with-trentos.md)
section in the TRENTO Handbook.

To run the **`rpi3_flasher`** the image has to be copied to the SD card together
with the Raspberry Pi boot files.

```shell
# copy boot files to SD Card
cp sdk/resources/rpi3_sd_card/* <sd_card_mount_point>/

# copy system image to mounted SD card
cp build-rpi3-Debug-flasher/images/os_image.elf <sd_card_mount_point>

# ensure linux file system caches are drained and
# everything is really written to the SDK card
sync

# unmount sd card, put it in the rpi3 and reboot
umount <sd_card_mount_point>
```

Now the SD card can be inserted into the RPi3B+.

Perform a board reset to boot the image and run **`rpi3_flasher`**.

Output example:

```console
U-Boot 2020.07-rc3-00002-g4b6f96a0a6 (May 26 2020 - 18:38:13 +0200)

DRAM:  948 MiB
RPI 3 Model B+ (0xa020d3)
MMC:   mmc@7e202000: 0, sdhci@7e300000: 1
Loading Environment from FAT... OK
In:    serial
Out:   vidconsole
Err:   vidconsole
Net:   No ethernet found.
starting USB...
Bus usb@7e980000: USB DWC2
scanning bus usb@7e980000 for devices... 4 USB Device(s) found
       scanning usb for storage devices... 0 Storage Device(s) found
Hit any key to stop autoboot:  0
5361808 bytes read in 231 ms (22.1 MiB/s)
## Starting application at 0x0084f000 ...

ELF-loader started on CPU: ARM Ltd. Cortex-A53 r0p4
  paddr=[84f000..d44ec7]
No DTB passed in from boot loader.
Looking for DTB in CPIO archive...found at 950560.
Loaded DTB from 950560.
   paddr=[37000..3afff]
ELF-loading image 'kernel'
  paddr=[0..36fff]
  vaddr=[e0000000..e0036fff]
  virt_entry=e0000000
ELF-loading image 'capdl-loader'
  paddr=[3b000..453fff]
  vaddr=[10000..428fff]
  virt_entry=17e48
Enabling MMU and paging
Jumping to kernel-image entry point...

Bootstrapping kernel
Booting all finished, dropped to user space
   INFO: /host/OS-SDK/pkg/components/SPI/SPI.c:34: SPI init
   INFO: /host/OS-SDK/pkg/components/SPI/Sspt_handle_irq@spt.c:172 handle irq called when no interrupt pending

PI.c:50: SPI init done
   INFO: /host/OS-SDK/pkg/components/RamDisk/RamDisk.c:160: RamDisk has size of 1048576 bytes
   INFO: /host/OS   INFO: /host/OS-SDK/pkg/components/Storage_Flash/Storage_Flash_SPI.c:519: SPI-Flash post init, timestmap 12138620-SDK/pkg/components/RamDisk/RamDisk.c:165: RamDisk is linked with image of 53468 bytes
000
   INFO: /host/OS-SDK/pkg/components/Storage_Flash/Storage_Flash_SPI.c:436: test flash offet=32256, timestmap 12159120000
   INFO: /host/OS-SDK/pkg/components/RamDisk/RamDisk.c:177: RamDisk was decompressed to 1048576 bytes
   INFO: /host/OS-SDK/pkg/components/Storage_Flash/Storage_Flash_SPI.c:574: SPI-Flash init done
Input storage reports size of 1048576 bytes
Output storage reports size of 8388608 bytes
Erasing: OK
Flashing: OK
Verifying: OK
Done.
```
