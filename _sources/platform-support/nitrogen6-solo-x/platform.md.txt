# Nitrogen6_SoloX Platform

## Subpages

```{toctree}
:maxdepth: 1
debugging_jtag
drivers
```


## Board Setup

To connect the board to the development PC, the following
cabling/tooling is required:

- AC/DC Power supply (5V, min 1.5A, DC barrel connector 2.1mm x
    5.5mm).
- A USB-to-RS232 cable for console and logs that needs to be connected
    to the serial connector labeled \"CONSOLE\" (note that it has an
    RS232\
    connector and can deal with RS232 voltage levels and not the
    UART/USB adapter that supports TTL voltage levels).
- An SD card that the U-Boot files need to be copied to as described
    in the section below.

The following components are optional but will make using the board much
easier:

- USB Hub.
- SD Card Switcher (see [Hardware Tools](../../development/hardware-tools.md)
    section) for quick accessing the SD card both from the board and the PC.
- USB Network adapter.

!["Nitrogen6_SoloX - Board Setup 1"](img/platform_adapt.jpg)

!["Nitrogen6_SoloX - Board Setup 2"](img/platform_eth-ports.jpg)

Once the USB-to-RS232 cable has been connected, start a serial monitor
that shows the received traffic. One way of doing this is to use
the **`picocom`** utility with the following command:

```shell
sudo picocom -b 115200 /dev/<ttyUSBX>
```

Hereby, **`<ttyUSBX>`** acts as a placeholder for the specific device
representing the USB-to-UART adapter, e.g. **`ttyUSB0`**. Note that
using **`sudo`** may not be required, this depends on your Linux group
membership giving your account access to **`/dev/<ttyUSBX>`**.

## Boot Mode

As with all platforms from BoundaryDevices, the Nitrogen6_SoloX can be
configured to boot from either USB (OTG port, see
<https://boundarydevices.com/unbricking-nitrogen6x-sabre-lite-i-mx6-board/>
(<https://boundarydevices.com/unbricking-nitrogen6x-sabre-lite-i-mx6-board/>))
or the internal SPI flash by setting the DIP switch (SW1) accordingly.

USB booting is typically only used when there is a failure in the SPI
flash resident bootloader. One may use USB boot to temporarily boot a
bootloader like U-Boot and reflash the SPI memory.

The exact procedure of how a bootloader such as U-Boot can be flashed to
the internal memory will be described in the sections below.

## Prerequisites

The Nitrogen6_SoloX with U-Boot flashed to its internal memory requires
additional binary files to be placed on the SD card for being able to
boot a TRENTOS system successfully.

The required files and their meaning are explained in the following
overview:

| Type    | Blob              | Usage                                                                                                                                                                      |
|---------|-------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| U-Boot  | u-boot.imx        | U-Boot binary which needs to be flashed to the internal NOR flash memory of the board.                                                                                     |
|         | 6x_bootscript.txt | U-Boot bootscript that is not placed on the SD card but is required to compile the 6x_bootscript from.                                                                     |
|         | 6x_bootscript     | Compiled U-Boot script from the 6x_bootscript.txt that needs to be placed on the SD card so U-Boot can find it once it boots up and execute the commands compiled into it. |
| TRENTOS | os_image.elf      | The TRENTOS system image, containing the TRENTOS OS (seL4 kernel & TRENTOS userland) and applications that along with the 6x_bootscript needs to be placed on the SD card. |

All of the U-Boot related files listed above can be found already
prepared in the **`resources/nitrogen6sx_sd_card`** folder of the SDK.

Please refer to [Using an SD Card with a TRENTOS System](../using-sd-card-with-trentos.md)
for a detailed description of how to partition the SD card properly.

In addition to that, the following sections explain in more detail, how
the required files can be created from scratch and where the sources to
build the binaries can be obtained from.

### U-Boot

**Info:** You may find already built **`u-boot.imx`** image in the
SDK's **`resources/nitrogen6sx_sd_card`**, so you may skip this paragraph.

### Preparations for Building U-Boot from Sources

If you want to build the required U-Boot binary directly from the
sources, first ensure that all the necessary packages are installed:

```shell
sudo apt install gcc-arm-linux-gnueabihf u-boot-tools flex bison
```

U-Boot is available in two versions:

- The mainline repository, that can be found at
    <https://github.com/u-boot/u-boot>
- The U-Boot repository from BoundaryDevices, see
    <https://github.com/boundarydevices/u-boot-imx6>

The mainline repository of U-Boot currently does not include support for
the Nitrogen6_SoloX and therefore the following sections will only
describe the build process for the sources gathered from
BoundaryDevices.

### Building U-Boot from BoundaryDevices Fork

Clone the U-Boot repository from BoundaryDevices to gather the required
sources.

```shell
git clone https://github.com/boundarydevices/u-boot-imx6.gitcd u-boot-imx6
```

At the time of writing, the current stable branch
is **`boundary-v2018.07`** (and head
is **`8a288f8beb9a00dea078d95830dda066a050a176`**). However, building a
working U-Boot requires the commit
**`995eab8b5b580b67394312b1621c60a71042cd18`** to be reverted. The
default U-Boot will not succeed to boot a seL4 image due to cache
configuration issues in the ELFLoader. Therefore, caches have to be
disabled in U-Boot before loading seL4, which is done by reverting the
mentioned commit before building U-Boot.

```shell
git checkout boundary-v2018.07git revert 995eab8b5b580b67394312b1621c60a71042cd18
```

Build the U-Boot binary with the default configuration for the Nitrogen6_SoloX.

```shell
make nitrogen6sx_defconfigmake ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2
```

After the build ran successfully, the folder should now contain the U-Boot
binary named **`u-boot.imx.`**

### Prepare a U-Boot Boot Script to Start a TRENTOS Image

U-Boot supports storing commands or command sequences in a plain text
file, which then later can be converted into a script image that U-Boot
can execute. This feature can be used to create a bootscript that will
instruct U-Boot to search for the system image that is supposed to be
run and to which address in the internal memory the image should be
loaded to.

**Info:** Both boot script **`6x_bootscript.txt`** and its compiled variant
**`6x_bootscript`** can be found in the SDK's **`resources/nitrogen6sx_sd_card`**,
however if you wish to create your own please follow belows instructions.

Create an empty text file, name it **`6x_bootscript.txt`**, and write
the following instructions to it. As mentioned, this will instruct
U-Boot to search the MMC devices for a system image called
**`os_image.elf`** and load it to the address **`0x83800000`**.

The DDR RAM of the i.MX6SX is mapped to the physical address space at
**`0x80000000`**. Therefore the TRENTOS **`os_image.elf`** will be
placed with an offset of **`0x03800000`** (effectively 58 MiB), leaving
the ElfLoader enough space to unpack without running the risk of
overwriting itself.

```bash
# expecting "board=nitrogen6sx, cpu=6SX"

echo "board=${board}, cpu=${cpu}"

if itest.s "nitrogen6sx" != "${board}" ; then
    echo "unsupported board";
    exit;
elif itest.s "6SX" != "${cpu}" ; then
    echo "unsupported cpu";
    exit;
fi

# DDR RAM base address

addr_base=0x80000000

# load to DDR RAM base at offset 56 MiB

setexpr addr_load  ${addr_base} + 0x3800000

# expecting "dtype=mmc, disk=1, bootpart=1, bootdir="

echo "dtype=${dtype}, disk=${disk}, bootpart=${bootpart}, bootdir=${bootdir}"

if itest.s "" == "${dtype}" ; then
    dtype=mmc
fi

if itest.s "" == "${disk}" ; then
    disk=0
fi

if itest.s "" == "${bootpart}" ; then
    bootpart=1
fi

if itest.s "" == "${bootdir}" ; then
    bootdir=/
fi

echo "loading os_image.elf at 0x${addr_load}..."
if load ${dtype} ${disk}:${bootpart} ${addr_load} ${bootdir}os_image.elf ; then
    bootelf ${addr_load}
fi

echo "Error loading os_image.elf"
```

Compile the bootscript using the **`mkimage`** tool from the u-boot-tools-package.

```shell
mkimage -A arm -O linux -T script -C none -n "boot script" \
-a 0 -e 0 -d \
6x_bootscript.txt \
6x_bootscript
```

Once the tool ran, the folder should now contain the compiled **`6x_bootscript.`**

### Boot from the Internal NOR Flash

#### Overview

As previously mentioned, the Nitrogen6_SoloX can boot either from USB
(*Serial Boot*) or its internal 2 MiB SPI NOR flash (*Serial EEPROM*).

This section describes how U-Boot can be flashed to the internal memory
of the board.

Please note that the following steps are all based on the assumption
that the board already contains a working U-Boot version in its internal
memory and that this preexisting version is supposed to be overwritten
with a different version.

**Info:** In case the board is not booting at all, first follow USB
Recovery paragraph steps below.

### Install U-Boot in the Board\'s SPI Flash

Copy the **`upgrade.scr`** and **`u-boot.imx`** file located in the
SDKs **`resources/nitrogen6sx_sd_card`** folder to the SD card.

Power on the board while in a terminal window running **`picocom`** as
described above.  Interrupt the boot process by pressing \<Enter\> when
the console outputs \"Hit any key to stop autoboot\".

```console
U-Boot 2018.07-36536-gb89e53e64c (Aug 31 2020 - 17:16:55 +0200)
CPU: Freescale i.MX6SX rev1.3 at 792 MHz
Reset cause: WDOG
Board: nitrogen6sx
I2C: ready
DRAM: 1 GiB
MMC: FSL_SDHC: 0, FSL_SDHC: 1
Loading Environment from SPI Flash...
SF: Detected sst25vf016b with page size 256 Bytes, erase size 4 KiB, total 2 MiB
OK
Display: lvds:tm070jdhg30 (1280x800)
In: serial
Out: serial
Err: serial
Net: AR8035 at 4
AR8035 at 5
FEC0 [PRIME], FEC1, usb_ether
Hit any key to stop autoboot: 1
```

Run the following command to upgrade U-Boot in the board\'s SPI flash
(note **`u`** at the end):

```console
=> run upgradeu
```

This will run the **`upgrade.scr`** script, search for the
**`u-boot.imx`** file and flash it. Once the process has successfully
completed, the SPI flash of the board should now contain the new version
of U-Boot. On success you should see the following output:

```console
=> run upgradeu
switch to partitions #0, OK
mmc0 is current device
Scanning mmc 0:1...
Found U-Boot script /upgrade.scr
4199 bytes read in 25 ms (163.1 KiB/s)
## Executing script at 80008000
Called gpio_direction_output() in mxc_gpio.c for gpio = 48 with value = 1.
SF: Detected sst25vf016b with page size 256 Bytes, erase size 4 KiB, total 2 MiB
probed SPI ROM
check U-Boot
490496 bytes read in 51 ms (9.2 MiB/s)
read 77c00 bytes from SD card
device 0 offset 0x400, size 0x77c00
SF: 490496 bytes @ 0x400 Read: OK
byte at 0x82000407 (0x17) != byte at 0x82400407 (0x87)
Total of 7 byte(s) were the same
Need U-Boot upgrade
Program in 5 seconds
5
4
3
2
1
erasing
SF: 786432 bytes @ 0x0 Erased: OK
programming
device 0 offset 0x800, size 0x77800
SF: 489472 bytes @ 0x800 Written: OK
device 0 offset 0x400, size 0x400
SF: 1024 bytes @ 0x400 Written: OK
verifying
device 0 offset 0x400, size 0x77c00
SF: 490496 bytes @ 0x400 Read: OK
Total of 490496 byte(s) were the same
---- U-Boot upgraded. reset
---- U-Boot upgraded. reset
---- U-Boot upgraded. reset
---- U-Boot upgraded. reset
---- U-Boot upgraded. reset
```

You may now do a power-on reset, to start U-Boot from the internal
memory.

### USB Recovery

If something went wrong, you can still recover the board via USB
recovery mode using the OTG port.

The steps below are based on
<https://boundarydevices.com/recovering-i-mx-platforms-using-uuu/>.

Force a boot to the USB recovery mode (OTG port) by setting the DIP
switch (SW1) to the ON position
(<https://boundarydevices.com/unbricking-nitrogen6x-sabre-lite-i-mx6-board/>).

Connect the micro-B USB cable, power on the board, and check if the
device gets detected:

```shell
lsusb | grep Free
Bus 001 Device 097: ID 15a2:0071 Freescale Semiconductor, Inc.
```

Download the latest **`uuu`** binary from
<https://github.com/NXPmicro/mfgtools/releases> and run **`uuu`** with the
desired U-Boot image:

```shell
sudo ./uuu u-boot.imx
uuu (Universal Update Utility) for nxp imx chips -- libuuu_1.4.43-0-ga9c099a
Success 1 Failure 0
1:21 2/ 2 [Done ] SDP: done
```

On success you should see on the U-Boot console:

```console
U-Boot 2018.07-36536-gb89e53e64c (Aug 31 2020 - 17:16:55 +0200)
CPU: Freescale i.MX6SX rev1.3 at 792 MHz
Reset cause: WDOG
Board: nitrogen6sx
I2C: ready
DRAM: 1 GiB
MMC: FSL_SDHC: 0, FSL_SDHC: 1
Loading Environment from SPI Flash...
SF: Detected sst25vf016b with page size 256 Bytes, erase size 4 KiB, total 2 MiB
OK
Display: lvds:tm070jdhg30 (1280x800)
In: serial
Out: serial
Err: serial
Net: AR8035 at 4
AR8035 at 5
FEC0 [PRIME], FEC1, usb_ether
Hit any key to stop autoboot: 1
```

Connect the SD card with the desired U-Boot image and the update script,
and run **`upgrade.scr`** as in the previous paragraph.

## Boot Up

Copy the **`6x_bootscript`** that was created as described above and
an **`os_image.elf`** system image to the SD card and insert it into the
microSD slot of the board.

**Note:** In case that an SD Card Switcher is utilized, be aware that
the microSD card slot of the Nitrogen6_SoloX does not hold the switcher
very well. If U-Boot is unable to find the bootscript, double-check the
SD card mounting in the slot. Consider using a small stripe of tape to
fix the SD Card Switcher to the microSD slot.

```console
MMC: no card present
mmc_init: -123, time 1
switch to partitions #0, OK
mmc1(part 0) is current device
** No partition table - mmc 1 **
```

The board can now be powered on and a result similar to the one below
shall be printed on the terminal:

```console
U-Boot 2018.07-36629-g4c8bf7a732 (Nov 12 2020 - 14:11:44 +0100)
CPU: Freescale i.MX6SX rev1.3 at 792 MHz
Reset cause: POR
Board: nitrogen6sx
I2C: ready
DRAM: 1 GiB
MMC: FSL_SDHC: 0, FSL_SDHC: 1
Loading Environment from SPI Flash...
SF: Detected sst25vf016b with page size 256 Bytes, erase size 4 KiB, total 2 MiB
OK
Display: lcd:1280x720M@60 (1280x720)
In: serial
Out: serial
Err: serial
Net: AR8035 at 4
AR8035 at 5
FEC0 [PRIME], FEC1, usb_ether
Hit any key to stop autoboot: 0
switch to partitions #0, OK
mmc0 is current device
Scanning mmc 0:1...
Found U-Boot script /6x_bootscript
1180 bytes read in 16 ms (71.3 KiB/s)
## Executing script at 80008000
8879004 bytes read in 438 ms (19.3 MiB/s)
CACHE: Misaligned operation at range [80ba7000, 80ba7034]
CACHE: Misaligned operation at range [80ba8000, 80bb23e0]
CACHE: Misaligned operation at range [80bb23e0, 80bb2f53]
CACHE: Misaligned operation at range [813ff040, 813ff078]
CACHE: Misaligned operation at range [813ff078, 813ff0a4]
## Starting application at 0x80ba7000 ...
ELF-loader started on CPU: ARM Ltd. Cortex-A9 r2p10
paddr=[80ba7000..813ff0a3]
No DTB passed in from boot loader.
Looking for DTB in CPIO archive...found at 80cb6384.
Loaded DTB from 80cb6384.
paddr=[8003d000..80040fff]
ELF-loading image 'kernel'
paddr=[80000000..8003cfff]
vaddr=[e0000000..e003cfff]
virt_entry=e0000000
```
