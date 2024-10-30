# RamDisk Generator Tool

## General

The RamDisk Generator Tool (**`rdgen`**) can be used to convert a file
system image produced by the
[Configuration Provisioning Tool](configuration-provisioning-tool.md)
(**`cpt`**) into a file that can be linked into the
[RamDisk](../components/ram-disk.md) component in order to pre-provision it.

The tool is used in the workflow to flash a configuration onto the
RPi3B+ SPI flash with the help of the
[Flasher Tool for RPi3B+](../tools/flasher-tool-rpi3b+.md).

## Build

The tool is already contained pre-built in the **`bin`** folder as part of the
delivered SDK.

In case a recompilation is required, it can be built manually in the following
way:

```shell
cd <sdk_root_directory>

# run sdk/tools/rdgen/build.sh in the container,
# parameter is the relative path to the SDK (within the container)
sdk/scripts/open_trentos_build_env.sh sdk/tools/rdgen/build.sh sdk
```

This will create a separate folder **`build_rdgen`**, which includes the
application binary.

## Run

The tool can be invoked from the command line and needs two parameters:

```shell
sdk/bin/rdgen <img> <out>
```

The tool reads the file specified by **`<img>`**, compresses it, and
writes an output file to **`<out>`**.

## Example

If as an example the file **`nvm_06`** should be converted into
**`flash.c`** the tool would be invoked as follows:

```shell
./rdgen nvm_06 flash.c
```

```console
rdgen: Compress NVM image into RLE encoded RamDisk format

Original size:        1048576 bytes
Compressed size:        53468 bytes (5.0991%)
```

The resulting file can be linked into the RamDisk component, as it
contains the symbols expected by the RamDisk. An example of a resulting
output file is as follows:

```c
#include <stdint.h>
#include <stddef.h>
//
// Generated with rdgen
// Original was 1048576 bytes, now just 53468 bytes (5.0991%)
//
uint8_t RAMDISK_IMAGE[] = {
    0x52,0x4c,0x45,0x00,0x00,0x10,0x00,0x01,0x01,0x03,0x00,0x01,0xf0,0x01,0x0f,
    0x01,0xff,0x01,0xf7,0x01,0x6c,0x01,0x69,0x02,0x74,0x01,0x6c,0x01,0x65,0x01,
    0x66,0x01,0x73,0x01,0x2f,0x01,0xe0,0x01,0x00,0x01,0x10,0x02,0x00,0x01,0x02,
    0x02,0x00,0x01,0x10,0x03,0x00,0x01,0x01,0x02,0x00,0x01,0xff,0x03,0x00,0x03,
    0xff,0x01,0x7f,0x01,0xfe,0x01,0x03,0x02,0x00,0x01,0x70,0x01,0x1f,0x01,0xfc,
    ...
    0x01,0x66,0x01,0xff,0x01,0x7f,0x03,0x00,0x01,0x34,0x01,0xf8,0x01,0x8a,0x01,
    0xfc,0x01,0x55,0x02,0x00,0x01,0x80,0x01,0x45,0x01,0xf8,0x01,0x66,0x01,0xff,
    0x01,0x7f,0x06,0x00,0x81,0x89,0xc0,0xff,
};
size_t RAMDISK_IMAGE_SIZE = sizeof(RAMDISK_IMAGE);
```
