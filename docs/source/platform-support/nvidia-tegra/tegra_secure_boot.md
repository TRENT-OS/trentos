# Tegra Secure Boot

## General

### Definitions

| Type | Description |
|------|-------------|
| ARM Trusted Firmware (ATF) |  |
| Generalized Security Carveout (GSC) | A type of aperture/carveout with configurable functionality that designates a region of the physical address space. Access from different clients to the region are controlled by registers in the Memory Controller (MC). |
| Trusted OS | - up to L4T 32.7.3: Trusty TEE OS <br> - since L4T 34.1: Linaro OP-TEE |

### Setup
A combination of

- ARM Trusted Firmware (ATF)
- a Trusted OS

A general overview about the setup is provided by the presentation "Jetson Security and Secure Boot".

## Trusted OS
### Nvidia Trusted Little Kernel (TLK)
- only for ARMv7-A
- source code: https://nv-tegra.nvidia.com/r/admin/repos/3rdparty/ote_partner/tlk
- more information
    - https://www.w3.org/2012/webcrypto/webcrypto-next-workshop/papers/webcrypto2014_submission_25.pdf 
    - https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/components/spd/tlk-dispatcher.rst

### Trusty TEE OS
TODO

### Linaro OP-TEE
TODO

## Platforms
### Tegra X1
On the Tegra X1 platform, only ATF (and Monitor), but no Trusted OS (e.g. no Trusty OS).

#### TOS
- TOS image size: 53680B
    - Monitor size: 53680B
    - OS size: 0B

#### Memory Layout
The memory layout is based on the setup of L4T 32.6.1.

| Software | Processor | Aperture | Memory Range | Size | Binary | Size | Entry Address | Used by |
|----------|-----------|----------|--------------|------|--------|------|---------------|---------|
| TegraBoot | BPMP (ARM7)	 | SecureOs Carveout	 | 0xff800000 - 0x100000000	 | 0x800000 [8 MiB]	 | unclear, probably <br> /Linux_for_Tegra/bootloader/tos-mon-only.img | 53680B |  |  |
|  |  | Lp0 Carveout	 | 0xff800000 | 0 | No BPMP FW loaded |  |  |  |
|  |  | BpmpFw Carveout	 | 0xff800000 | 0 | No BPMP FW loaded |  |  |  |
|  |  | GSC1 Carveout	 | 0xff800000 | 0 |  |  |  |  |
|  |  | GSC2 Carveout	 | 0xff900000 | 0x100000 |  |  |  |  |
|  |  | GSC4 Carveout |  | 0xff800000 | 0 |  |  |  |
|  |  | GSC5 Carveout |  | 0xff800000 | 0 |  |  |  |
|  |  | GSC3 Carveout	 | 0xbf800000 | 0x200000 |  |  |  |  |
|  |  | RamDump Carveout	 | 0x0 |  | 0 |  |  |  |
|  |  | Nck Carveout	 | 0xff800000 | 0 |  |  |  |  |
|  |  | Tboot-CPU	 | 0xa0000000 | - | /Linux_for_Tegra/bootloader/nvtboot_cpu.bin | 65760B [64,2 KiB]	 | 0xa0000258 |  |
|  |  | CBoot | 0x92c00000 | - | /Linux_for_Tegra/bootloader/t210ref/cboot.bin | 485392B [474 KiB]	 | 0x92c00258 |  |
|  |  | NvTbootBootloaderDTB | 0x83000000 | - | /Linux_for_Tegra/bootloader/nvtboot_cpu.bin | 228986B [223,6 KiB]	 |  | CBoot |
|  |  | NvTbootKernelDTB | 0x83100000 | - | /Linux_for_Tegra/bootloader/t210ref/cboot.bin | 228986B [223,6 KiB]	 | | U-Boot |
| CBoot | CCPLEX | RP4 XUSB firmware (since L4T 32.7.1 loaded by U-Boot) | 0x92ca828c | 126464B | /Linux_for_Tegra/bootloader/kernel_tegra210-p3448-0003-p3542-0000.dtb |  |  |  |
|  |  | U-Boot | 0x80080000 - ... <br> (defined by variable CONFIG_SYS_TEXT_BASE in <br> file configs/p3541-0000_defconfig in U-Boot source code) | - | /Linux_for_Tegra/bootloader/t210ref/p3541-0000/u-boot.bin | 663488B [647,9 KiB] | 0x80080000 |  |
| U-Boot | CCPLEX | U-Boot configuration scripts/files <br> Linux kernel <br> TRENTOS | 0x83000000 (fdt_addr_r)<br> - 0x83200000 (ramdisk_addr_r)<br> - 0x84000000 (kernel_addr_r)<br> - 0x90000000 (scriptaddr)<br> - 0x90100000 (pxefile_addr_r)<br> - 0x90200000 (fdtoverlay_addr_r)<br> (defined in include/configs/tegra210-common.h)<br> -0x90000000 (custom set via setenv bootcmd) | variable (depending on built system) | loaded from partition /boot within rootfs <br> loaded via TFTP server |  |  |
| TRENTOS | CCPLEX | Elfloader <br> seL4 kernel <br> capDL loaded | - | - | - | - | - | - |



| Software | Processor | Aperture | Memory Range | Size | Binary | Size | Entry Address | Used by |
|----------|-----------|----------|--------------|------|--------|------|---------------|---------|
| MB2 (TBoot-BPMP)	 | BPMP (ARM Cortex-R5)	 | partition bpmp-fw	 | 0xd7800000	 |  | "Binary(16) of size 534416"	 |  |  |  |
|  |  | partition bpmp-fw-dtb	 | 0xd79f0000	 |  | "Binary(17) of size 113200 at 0xd79e4400"	 |  |  |  |
|  |  | SCE-FW (partition sce-fw) <br> → Init SCE | 0xd7300000 |  | "Binary(12) of size 125632"	 |  |  |  |
|  |  | APE-FW (partition adsp-fw) <br> → copy BTCM section | 0xd7400000 |  | "Binary(11) of size 77216"	 |  |  |  |
|  |  | partition cpu-bootloader | 0x96000000 |  | "Binary(13) of size 308800"	 |  |  |  |
|  |  | partition bootloader-dtb | 0x8520a400 |  | "Binary(20) of size 192320"	 |  |  |  |
|  |  | partition secure-os | 0x8530a600 |  | "Binary(14) of size 402864"	 |  |  |  |
|  |  | TOS boot-params | 0x85000000 |  |  |  |  |  |
|  |  | EKS (partition eks) | 0x8590a800	 |  | "Binary(15) of size 1040"	 |  |  |  |
|  |  | boot profiler | 0x175844000	 |  |  |  |  |  |
|  |  | boot profiler for TOS | 0x175844000  |  |  |  |  |  |
|  |  | unhalting SCE |  |  |  |  |  |  |
|  |  | primary memory - start: 0x80000000, size: 0x70000000 |  |  |  |  |  |  |
|  |  | extended memory - start: 0xf0110000, size: 0x856f0000 |  |  |  |  |  |  |
| CBoot | CCPLEX | CPU-BL Params | 0x175800000 |  |  |  |  |  |
|  |  | Region 1 | 0x177f00000 | 0x00100000 |  |  |  |  |
|  |  | Region 2 | 0x177e00000 | 0x00100000 |  |  |  |  |
|  |  | Region 3 | 0x177d00000 | 0x00100000 |  |  |  |  |
|  |  | Region 4 | 0x177c00000 | 0x00100000 |  |  |  |  |
|  |  | Region 5 | 0x177b00000 | 0x00100000 |  |  |  |  |
|  |  | Region 6 | 0x177800000 | 0x00200000 |  |  |  |  |
|  |  | Region 7 | 0x177400000 | 0x00400000 |  |  |  |  |
|  |  | Region 8 | 0x177a00000 | 0x00100000 |  |  |  |  |
|  |  | Region 9 | 0x177300000 | 0x00100000 |  |  |  |  |
|  |  | Region 10 | 0x176800000 | 0x00800000	 |  |  |  |  |
|  |  | Region 11 | 0x30000000 | 0x00040000 |  |  |  |  |
|  |  | Region 12 | 0xf0000000 | 0x00100000 |  |  |  |  |
|  |  | Region 13 | 0x30040000 | 0x00001000 |  |  |  |  |
|  |  | Region 14 | 0x30048000 | 0x00001000 |  |  |  |  |
|  |  | Region 15 | 0x30049000 | 0x00001000 |  |  |  |  |
|  |  | Region 16 | 0x3004a000 | 0x00001000 |  |  |  |  |
|  |  | Region 17 | 0x3004b000 | 0x00001000 |  |  |  |  |
|  |  | Region 18 | 0x3004c000 | 0x00001000 |  |  |  |  |
|  |  | Region 19 | 0x3004d000 | 0x00001000 |  |  |  |  |
|  |  | Region 20 | 0x3004e000 | 0x00001000 |  |  |  |  |
|  |  | Region 21 | 0x3004f000 | 0x00001000 |  |  |  |  |
|  |  | Region 23 | 0xf0100000 | 0x00010000 |  |  |  |  |
|  |  | Region 28 | 0x84400000 | 0x00400000 |  |  |  |  |
|  |  | Region 29 | 0x30000000 | 0x00010000 |  |  |  |  |
|  |  | Region 30 | 0x178000000 | 0x08000000 |  |  |  |  |
|  |  | Region 32 | 0x176000000 | 0x00600000 |  |  |  |  |
|  |  | Region 33 | 0x80000000 | 0x70000000 |  |  |  |  |
|  |  | Region 34 | 0xf0110000 | 0x856f0000 |  |  |  |  |
|  |  | Region 37 | 0x1772e0000 | 0x00020000 |  |  |  |  |
|  |  | Region 38 | 0x84000000 | 0x00400000 |  |  |  |  |
|  |  | Region 39 | 0x96000000 | 0x02000000 |  |  |  |  |
|  |  | Region 40 | 0x85000000 | 0x01200000 |  |  |  |  |
|  |  | Region 41 | 0x175800000 | 0x00500000 |  |  |  |  |
|  |  | reserved memory for U-Boot relocation | 0xfbe00000 |  |  |  |  |  |
|  |  | partition kernel-dtb | 0x80000000 |  |  |  |  |  |
|  |  | partition kernel-bootctrl | 0xa8000000 |  |  |  |  |  |
|  |  | boot image load address | 0x80400000 | 99085 |  |  |  |  |
|  |  | partition kernel | 0x80400000 |  |  |  |  |  |
|  |  | kernel hdr | 0x80400000 |  |  |  |  |  |
|  |  | kernel dtb | 0x80000000 |  |  |  |  |  |
|  |  | copy kernel image from 0x80400000 to ... | 0x80600000 | 626821 |  |  |  |  |
|  |  | move ramdisk from 0x8049a000 to ... | 0x947d0000 | 0 |  |  |  |  |
|  |  | update memory info to DTB - <br> add to section /memory in DTB | 0x80000000 | 0x70000000 |  |  |  |  |
|  |  | update memory info to DTB - <br> add to section /memory in DTB | 0xf0200000 | 0x85600000 |  |  |  |  |
|  |  | update memory info to DTB - <br> add to section /memory in DTB | 0x175e00000 | 0x200000 |  |  |  |  |
|  |  | update memory info to DTB - <br> add to section /memory in DTB | 0x176600000 | 0x200000 |  |  |  |  |
|  |  | update memory info to DTB - <br> add to section /memory in DTB | 0x177000000 | 0x200000 |  |  |  |  |
|  |  | U-Boot | 0x80080000 - ... <br> (defined by variable CONFIG_SYS_TEXT_BASE <br> in file configs/p3636-0001_defconfig in U-Boot source code) | - | /Linux_for_Tegra/bootloader/t186ref/p3636-0001/u-boot.bin |  | 0x80080000 |  |
| U-Boot | CCPLEX | U-Boot configuration scripts/files <br> Linux kernel <br> TRENTOS | 0x82000000 (fdt_addr_r) <br> - 0x82100000 (ramdisk_addr_r)<br> - 0x80080000 (kernel_addr_r)<br> - 0x90000000 (scriptaddr)<br> - 0x90100000 (pxefile_addr_r)<br> - 0x90200000 (fdtoverlay_addr_r) <br> (defined in include/configs/tegra186-common.h)<br> 0x8590a800  (custom set via setenv bootcmd) <br> → has to be adapted to another address (maybe 0x90000000 like for Jetson Nano ???) | variable (depending on built system) | loaded from partition /boot within rootfs <br> loaded via TFTP server |  |  |  |
| TRENTOS | CCPLEX | Elfloader <br> seL4 kernel <br> capDL loaded |  |  |  |  |  |  |

Linux detects the following free memory regions:

- 0x80000000 - 0xf0000000 (size 0x70000000)
- 0xf0200000 - 0x175800000 (size 0x85600000)
- 0x175e00000 - 0x176000000 (size 0x200000)
- 0x176600000 - 0x176800000 (size 0x200000)
- 0x177000000 - 0x177200000 (size 0x200000)

### Tegra Xavier
On the Tegra Xavier platform, a combination of ATF and, depending on the L4T revision, either Trusty OS or Linaro OP-TEE.

### Tegra Orin
On the Tegra Orin platform, a combination of ATF and Linaro OP-TEE.









