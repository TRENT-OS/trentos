# NVidia Tegra X1


```{toctree}
:maxdepth: 1
nvidia_tegra_x1_bootflow
nvidia_tegra_x1_boot_setup
```


## Scope
- This page shall be used to provide an overview of all work packages that are necessary for the porting of TRENTOS to the Nvidia Tegra X1 SoC.
- Main focus here is on supporting the specific Tegra X1 version that is used within the Nvidia Jetson Nano platform.
- for a general overview see also
    - https://en.wikipedia.org/wiki/Tegra
    - https://en.wikipedia.org/wiki/Nvidia_Jetson

## Hardware
### Tegra X1 (T210)

The following provides an overview about the default Nvidia Tegra X1 SoC configuration:

- CPU: 4x ARM Cortex-A57 + 4x ARM Cortex-A53, with the 4x ARM Cortex-A53 cores remaining unused/not accessible from an OS side
- RAM: up to 8 GiB LPDDR4
- GPU: 256-core Maxwell

The Jetson Nano uses a reduced version of the Nvidia Tegra X1 SoC, containing the following hardware configuration:

- CPU: 4x ARM Cortex-A57
- RAM: up to 4 GiB LPDDR4
- GPU: 128-core Maxwell

## Variants of Nvidia Tegra X1

| Jetson Nano Version              | Revision                                                                    | Reference Carrier Board  | Jetson Module                        | Compatible property values |
|--------------------------------|------------------------------------------------------------------------------|--------------------------|-----------------------------------|--------------------------|
| Jetson TX1 Developer Kit         | P2371-2180                                                                  | P2597-0000               | P2180-1000                           | nvidia,p2597-0000+p2180-1000 |
| Jetson Nano 2GB Developer Kit    | 945-13541-0000-000 (P3541) [including 802.11ac wireless adapter and cable]  | P3542-0000               | P3448-0003 (includes 1x 2GIB QSPI)   | nvidia, p3542-0000+p3448-0003 |
|                                 | 945-13541-0001-000 (P3541) [NOT including adapter and cable]                | P3542-0000               | P3448-0003 (includes 1x 2GIB QSPI)   | nvidia, p3542-0000+p3448-0003 |
| Jetson Nano Developer Kit (A02)  | 945-13450-0000-000 (P3450)                                                  | P3449-0000               | P3448-0000 (includes 1x microSD)     | nvidia, p3449-0000-a02+p3448-0000-202 |
| Jetson Nano Developer Kit (B0x)  | 945-13450-0000-100 (P3450)                                                  | P3449-0000               | P3448-0000 (includes 1x microSD)     | nvidia.p3449-0000-b00+p3448-0000-600 |
| Jetson Nana SoM                               |                                                                    | P3449-0000               | P3448-0002 (includes 1x 16GiB eMMC) |                                 |


SBC
Nvidia offers the Jetson Nano in two SBC variants

- [Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit) (p3450)
    - 4 GiB RAM
    - 1x microSD
    - 1x 10/100/1000 MBit Ethernet
    - 1x M.2 Key E
    - 1x HDMI
    - 1x Display Port
    - 2x Camera Conn. (MIPI-CSI2)
    - 4x USB 3.0
    - 1x USB 2.0 Micro B
    - 40-pin Exp Header (GPIO, I2C, I2S, SPI, UART)
- [Jetson Nano 2GB Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-2gb-developer-kit) (p3451)
    - 2 GiB RAM
    - 1x microSD
    - 1x QSPI ???
    - 1x 10/100/1000 MBit Ethernet
    - 1x 802.11ac wireless
    - 2x USB 2.0 Type A
    - 1x USB 3.0 Type A
    - 1x USB Type C (Power Conn. 5V/3A)
    - 1x USB 2.0 Micro B
    - 1x HDMI Type A
    - 1x Camera Conn. (MIPI-CSI2)
    - 40-pin Exp Header (GPIO, 2x I2C, I2S, 2x SPI, UART)


## SoM
Nvidia also offers the [Jetson Nano SoM](https://developer.nvidia.com/embedded/jetson-nano) (p3448-0002) in a 4 GiB version:

- 4 GiB RAM
- 16 GiB eMMC 5.1
- 1x 10/100/1000 MBit Ethernet
- 1x M.2 Key E
- 1x HDMI
- 4x USB 3.0
- 1x USB 2.0 Micro B
- Camera Conn. (MIPI-CSI2)
- 40-pin Exp Header (GPIO, I2C, I2S, SPI, UART)

## Nvidia Developer Resources
- Nvidia JetPack SDK: https://developer.nvidia.com/jetpack-sdk-46
- Nvidia L4T
    - Latest (including Jetson Nano): https://developer.nvidia.com/embedded/linux-tegra-r3261
    - up to Jetson TK1: https://developer.nvidia.com/linux-tegra-r218
- Nvidia Jetson Linux Developer Guide: https://docs.nvidia.com/jetson/l4t/index.html
- Nvidia Jetson Nano Community
    - https://developer.nvidia.com/embedded/community/resources
    - https://forums.developer.nvidia.com/c/agx-autonomous-machines/jetson-embedded-systems/jetson-nano/76

### Jetson Nano Developer Kit
- User Guide: https://developer.nvidia.com/embedded/downloads#?search=Jetson%20Nano%20Developer%20Kit%20User%20Guide

### Jetson Nano 2GB Developer Kit
- User Guide: https://developer.nvidia.com/embedded/learn/jetson-nano-2gb-devkit-user-guide

## Third-Party
### Boot
- https://github.com/OE4T/u-boot-tegra
- https://github.com/OE4T/tegra-boot-tools

### Linux
- https://github.com/OE4T/linux-tegra-4.9
- https://github.com/OE4T/meta-tegra

## Existing seL4 support
The mainline seL4 already provides support for the Nvidia Tegra X1 in a limited form and based on specific development boards. According to https://docs.sel4.systems/Hardware/, seL4 currently supports the following platforms:

- Nvidia Tegra K1
    - [Jetson TK1](https://docs.sel4.systems/Hardware/jetsontk1.html) (Nvidia)
    - [TK1 SOM](https://docs.sel4.systems/Hardware/CEI_TK1_SOM/) (Colorado Engineering)
- [Jetson TX1](https://docs.sel4.systems/Hardware/jetsontx1.html) (Nvidia)
- [Jetson TX2](https://docs.sel4.systems/Hardware/JetsonTX2.html) (Nvidia)

### Resources
- Technical Reference Manual (TRM): https://developer.nvidia.com/embedded/downloads#?tx=$product,jetson_tx1 (after registration)
    - Tegra X1 TRM
    - Tegra X1 TRM
    - Nvidia Download Center: https://developer.nvidia.com/embedded/downloads

## Existing seL4 Device Drivers
The seL4 already provides basic support for the Jetson TK1, the Jetson TX1 and the Jetson TX2 in a limited form. Support for the Jetson Nano SBC is generally based on the Jetson TX1 SoC, from which baseline support can be adopted.

### Elfloader

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /tools/seL4/cmake-tool/helpers/application_settings.cmake  | -                                                                                                                          | good    | adapt                  |
| /tools/seL4/cmake-tool/helpers/simulation.cmake            | not required, as there doesn't exist a respective QEMU platform                                                            | okay    | nothing to do          |
| /tools/seL4/elfloader-tool/src/arch-arm/smp_boot.c         | -                                                                                                                          | unknown | check existing support |
| /tools/seL4/elfloader-tool/src/arch-arm/drivers/smp-psci.c | driver checks for compatibility String arm, psci-1.0, which is available in to be added Nvidia Jetson Nano .dts file       | unknown | check existing support |
| /tools/seL4/elfloader-tool/src/drivers/uart/8250-uart.c    | driver checks for compatibility String nvidia, tegra20-uart, which is available in to be added Nvidia Jetson Nano.dts file | good    | check existing support |
| /tools/seL4/elfloader-tool/src/plat/tx1/platform_init.c    | -                                                                                                                          | good    | check existing support |
| /tools/seL4/elfloader-tool/src/plat/tx1/monitor.S          | adapt if required (see Jetson TK1)                                                                                         | unknown |                        |

### seL4

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /kernel/configs/seL4Config.cmake	  | check if platform config is required (ARMv8-A, Cortex-A57, SMMU already there)	 | good    | check existing support |
| /kernel/include/drivers/smmu/smmuv2.h	  | check if additional config beyond standard ARM SMMUv2 is required	 | good    | check existing support |
| /kernel/include/drivers/timer/arm_generic.h	  | check if additional config beyond standard ARM Generic Timer is required	 | good    | check existing support |
| /kernel/libsel4/sel4_plat_include/tx1/sel4/plat/api/constants.h	  | basic stub for TX1 already exists, nevertheless has to be checked if there is a difference between Jetson TX1 Dev Kit and Jetson Nano SBC	 | good    | check existing support |
| /kernel/src/arch/arm/machine/gic_v2.c	  | driver is supporting ARM GICv2 standard, to which the TX1arm,cortex-a15-gic should adhere to	 | good    | check existing support |
| /kernel/src/arch/arm/config.cmake	  | check if platform config is required (Cortex-A57 already there)	 | good    | check existing support |
| /kernel/src/drivers/serial/tegra_omap3_dwapb.c	| driver checks for compatibility String nvidia,tegra20-uart, which is available in to be added Nvidia Jetson Nano .dts file	 | okay    | check existing support |
| /kernel/src/drivers/serial/config.cmake	  | includes the compatibility String nvidia,tegra20-uart for registering the driver available in tegra_omap3_dwapb.c	 | okay    | check existing support |
| /kernel/src/drivers/smmu/smmuv2.c	  | - | okay    | check existing support |
| /kernel/src/drivers/smmu/config.cmake	  | includes the compatibility String arm,mmu-500 for registering the driver available in smmuv2.c	 | okay    | check existing support |
| /kernel/src/drivers/timer/generic_timer.c	  | - | okay    | check existing support |
| /kernel/src/drivers/timer/config.cmake	  | includes compatibility String arm,armv8-timer for registering the driver available in generic_timer.c	 | okay    | check existing support |
| /kernel/src/plat/tx1/config.cmake	 | - a new platform (KernelPlatformTx1, PLAT_TX1) was added <br> - usage of the <br> - tools/dts/tx1/tx1.dts <br> - src/plat/tx1/overlay-tx1.dts <br> - references <br> - arch/machine/gic_v2.h <br> - drivers/timer/arm_generic.h <br> - currently missing references <br> - drivers/smmu/smmuv2.h <br> includes references to <br> - ARM Generic Interrupt Controller: <br> - src/arch/arm/machine/gic_v2.c <br> - ARM L2 Cache Controller: src/arch/arm/machine/l2c_nop.c <br> According to the ARM documentation, the Cortex-A15 GIC implements the GICv2 architecture. | okay | check existing support |
| /kernel/src/plat/tx1/overlay-tx1.dts	 | - | okay    | check existing support |
| /kernel/tools/dts/tx1.dts	 | .dts file for the Nvidia Jetson TX1, currently suited for the Jetson TX1 Dev Kit	 | okay    | check existing support |
| /kernel/tools/hardware.yaml	| - ARM PMU: arm,armv8-pmuv3 <br> - SMMU: arm,mmu-500 <br> - ARM Global Timer: arm,armv8-timer <br> - Serial: nvidia,tegra20-uart <br> - ARM PSCI: arm,psci-1.0 | okay    | check existing support |
| /kernel/tools/hardware/irq.py	  | check compatibility String in CONTROLLERS for <br> - arm,gic-400 <br> - arm,cortex-a15-gic <br> Question: which one is used here, as both variants a referenced in other files and .dts ??? | okay    | check existing support |



## Libraries
The seL4 libraries can be split into

- libplatsupportports (projects_libs) → a collection of drivers ported from GPL-based sources such as U-boot and Linux
- libsdhcdrivers (projects_libs)
- libtx2bpmp (projects_libs) → a port of the Tegra186 Boot and Power Management Processor (BPMP) interfaces from U-boot to seL4
- libusbdrivers (projects_libs)
- libsel4vm (sel4_projects_libs)
- libsel4vmmplatsupport (sel4_projects_libs)
- libplatsupport (sel4_util_libs)
- libethdrivers (sel4_util_libs)

### projects_libs
#### libplatsupportports


| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| libs/projects_libs/libplatsupportports/plat_include/tx2/platsupportports/plat/ | Includes header files for drivers specific to the Nvidia Tegra X2 and ported from Linux or U-boot <br> - GPIO (nvidia,tegra186-gpio) <br>- Pinmux (nvidia,tegra186-pinmux) | okay | check if adaptation is required for TX1 |
| /libs/projects_libs/libplatsupportports/src/plat/tx2/ | Includes drivers that are specific to the Nvidia Tegra X2 and ported from Linux or U-boot <br> - Clock (nvidia,tegra186-car) <br>     - libplatsupport clock ID to TX2 BPMP clock ID bindings <br>    - Clock driver for the TX2 CAR (Clock And Reset) controller which relies on libtx2bpmp <br> - GPIO <br>     - GPIO driver for the TX2 GPIO controller based off on U-boot's implementation <br> - Pinmux <br>    - Pin controller driver for the TX2 MUX based off on NVIDIA's L4T implementation <br> - Reset <br>     - libplatsupport reset ID to TX2 BPMP reset ID bindings <br>    - Reset driver for the TX2 CAR (Clock And Reset) controller which relies on libtx2bpmp | okay | check if adaptation is required for TX1 |

#### libsdhcdrivers
Currently, no Nvidia specific SDHCI driver is available. Maybe support from NXP i.MX6 could be adopted.

#### libtx2bpmp


| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /libs/projects_libs/libtx2bpmp/ | Includes header files for drivers and drivers that are specific to the Nvidia Tegra X2 and ported from U-boot <br> - BPMP <br>    - BPMP driver for the BPMP (Boot and Power Management Processor) co-processor on the TX2 based off on U-boot's implementation <br> - HSP <br>     - HSP driver for the HSP (Hardware Synchronisation Primitives) module on the TX2 <br> - IVC <br>     - IVC implementation for the IVC (Inter-VM Communication) protocol used for communicating with the BPMP | okay | check if adaptation is required for TX1 |


#### libusbdrivers
Currently, there is only support for the Jetson TK1. Nevertheless, USB might be similar on both Jetson TX1 and TX2. Current USB drivers seem to have USB EHCI support only.

### sel4_projects_libs
#### libsel4vm

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /libs/sel4_projects_libs/libsel4vm/src/arch/arm/vgic/vgic.c	 | check for CONFIG_PLAT_TX2	 | okay | check the functionality of the drivers provided |

#### libsel4vmmplatsupport

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /libs/sel4_projects_libs/libsel4vmmplatsupport/src/arch/arm/guest_image.c	 | check for CONFIG_PLAT_TX2 | okay | check the functionality of the drivers provided |

### sel4_util_libs
#### libethdrivers

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /libs/sel4_util_libs/libethdrivers/CMakeLists.txt		 | Specific to the Nvidia Tegra X2 | okay | check the functionality of the drivers provided |
| /libs/sel4_util_libs/libethdrivers/include/ethdrivers/raw.h | - | okay | check if adaptation is required |
| /libs/sel4_util_libs/libethdrivers/include/ethdrivers/tx2.h | Specific to the Nvidia Tegra X2 | okay | check the functionality of the drivers provided |
| /libs/sel4_util_libs/libethdrivers/src/plat/tx2	 | Wrapper around the U-boot Ethernet driver for the Nvidia Tegra X2 | okay | check the functionality of the drivers provided |

#### libplatsupport

| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /libs/sel4_util_libs/libplatsupport/CMakeLists.txt | - | okay | check if adaptation is required |
| /libs/sel4_util_libs/libplatsupport/include/platsupport/interface_types.h	 | TX2_BPMP_INTERFACE is mentioned here | okay | check if adaptation is required |
| /libs/sel4_util_libs/libplatsupport/mach_include/nvidia/platsupport/mach	 | currently there seems to be support for the TK1 | okay | check if adaptation is required specific to TX1 |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/clock.h	 | - | okay | check existing support, especially as TX2 has a specific one |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/gpio.h	 | currently there only exists a driver for the TK1	 | unknown | check if driver can be adopted from TK1 to TX1 |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/i2c.h	 | - | okay | check existing support |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/mux.h	 | currently there only exists a driver for the TK1	 | unknown | check if really required and in case yes, check if driver can be adopted from TK1 to TX1 |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/reset.h	 | currently there only exists a driver for the TX2	 | unknown | check if driver can be adopted from TX2 to TX1 |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/serial.h	 | - | okay | check existing support, especially as TX2 has a specific one |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/spi.h	 | currently there only exists a driver for the TK1 | unknown | check if driver can be adopted from TK1 to TX1 |
| /libs/sel4_util_libs/libplatsupport/plat_include/tx1/platsupport/plat/timer.h	 | - | okay | check existing support, especially as TX2 has a specific one |
| /libs/sel4_util_libs/libplatsupport/src/mach/nvidia/chardev.c	 | Includes driver that is generic for the general Nvidia Tegra architecture, meaning the Nvidia Tegra K1, the Nvidia Tegra X1 and the Nvidia Tegra X2 | okay | check existing support |
| /libs/sel4_util_libs/libplatsupport/src/mach/nvidia/ltimer.c	 | Includes driver that is generic for the general Nvidia Tegra architecture, meaning the Nvidia Tegra K1, the Nvidia Tegra X1 and the Nvidia Tegra X2 | okay | check existing support |
| /libs/sel4_util_libs/libplatsupport/src/mach/nvidia/serial.c	 | Includes driver that is generic for the general Nvidia Tegra architecture, meaning the Nvidia Tegra K1, the Nvidia Tegra X1 and the Nvidia Tegra X2 | okay | check existing support |
| /libs/sel4_util_libs/libplatsupport/src/mach/nvidia/timer.c	 | Includes driver that is generic for the general Nvidia Tegra architecture, meaning the Nvidia Tegra K1, the Nvidia Tegra X1 and the Nvidia Tegra X2 | okay | check existing support |
| /libs/sel4_util_libs/libplatsupport/src/plat/tx1/ | currently, there exists no such folder (only for TK1 and TX2) <br> - clock <br> - gpio <br> - i2c <br> - mux <br> - spi | unkown | check if drivers can be adopted from TK1 and TX2 |

### CAmkES

| Source                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
| /tools/camkes/libsel4camkes/src/sys_uname.c	 | only support for CONFIG_PLAT_TK1	 | okay | check if support is also required for the Nvidia Tegra X1 |

### Global Components

| Component | Source | Comment  | Status  | Task  |
|------------------------------------------------------------|----------------------------------------------------------------|----------------------------------------------------------|---------|------------------------|
| BPMPServer | /libs/sel4_global_components/plat_components/tx2/BPMPServer/	|  | okay | check the functionality of the drivers provided |
| ClockServer | /libs/sel4_global_components/components/ClockServer/include/plat/tx2/plat/clock.h	 | | okay | check the functionality of the drivers provided |
| Ethdriver | /libs/sel4_global_components/components/Ethdriver/include/plat/tx2/plat/eth_devices.h	 | |okay | check the functionality of the drivers provided |
| GPIOMUXServer | /libs/sel4_global_components/components/EGPIOMUXServer/include/plat/tx2/ | | okay | check the functionality of the drivers provided |
|  | /libs/sel4_global_components/components/EGPIOMUXServer/libGPIOMUXServer-client/plat_include/tx2/ | | okay | check the functionality of the drivers provided |
| ResetServer | /libs/sel4_global_components/components/ResetServer/include/plat/tx2/plat/reset.h | | okay | check the functionality of the drivers provided |
| SerialServer  | - | ARM generic, if serial driver support is provided via libplatsupport | okay | check the functionality of the drivers provided |
| TimeServer | /libs/sel4_global_components/components/TimeServer/include/plat/tx1/plat/timers.h | | okay | check the functionality of the drivers provided |


## Existing Linux and U-boot device drivers
TODO: check driver support from U-boot (Nvidia specific) and Linux (here L4T)

| HW Type | Connected via  | Nvidia Tegra X1 | U-boot | Linux |
|------------------------------------------------------------|-----------------------------------------|----------------------------------------------------------------------------------|---------|------------------------|
| Pinmux |  |  |  |  |
| Timer |  |  |  |  |
| Clock |  |  |  |  |
| UART |  |  |  |  |
| Interrupt |  |  |  |  |
| Reset |  |  |  |  |
| Ethernet MAC	 | PCI | 	Realtek RTL8111 (no public documentation available, only via third-party) | driver: r8169 | detected as: RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller <br> driver: r8169 (which is reported to be unstable, r8168-dkms recommended instead on the internet) |
| Ethernet PHY	 |  |  |  |  |
| WiFi/BT	 | external, via PCI/USB |  |  |  |
| SDHCI |  |  |  |  |
| (Optional) QSPI Flash	 |  |  |  |  |
| (Optional) eMMC |  |  |  |  |
| (Optional) GPIO	 |  |  |  |  |
| (Optional) I2C	 |  |  |  |  |
| (Optional) SPI	 |  |  |  |  |
| (Optional) USB	 |  |  |  |  |
| Additional |  |  |  |  |

## Porting Efforts
Baseline support for the Jetson TX1 is already available in seL4. Apart from that, seL4 also supports both the predecessor (Jetson TK1) and the successor (Jetson TX2) in a basic fashion. Support from these platforms might also be taken over to extend the existing Jetson TX1 port. The following parts of the seL4 ecosystem would be affected:

- Elfloader
    - base support available for TX1 (and other Nvidia platforms)
    - no real adaptation required
- seL4
    - Generic ARM support for Cortex-A57 cores, ARM Generic Timer, ARM GICv2 and Nvidia UART available
    - SMMU support has to be checked (but support for Jetson TX2 available)
    - separate .dts file for Jetson Nano might have to be provided, as existing TX1 support is suited to Jetson TX1 Dev Kit only
- libplatsupport
    - Clock, I2C, Serial and Timer drivers are available for the Jetson TX1
    - GPIO, Mux and SPI drivers might be adopted from Jetson TK1
    - Reset driver might be adopted from Jetson TX2
- libethdrivers
    - Ethernet driver support would require PCIe driver support, as the Realtek network hardware device is coupled via PCIe
    - A driver for the Jetson TX2 NIC is available (ported from U-boot), but it seems as if this is a native Ethernet now (incompatibility to previous ones in Jetson TK1 and Jetson TX1)
    - ...
- libsdhcdrivers
    - currently, no support is available for Nvidia specific SDHCI support
    - potentially take over and adapt the existing NXP i.MX6 SDHCI driver structure, in case it is SDHCI compliant in general
- libusbdrivers
    - currently, there is only support for the Jetson TK1; nevertheless, USB might be similar on both Jetson TX1 and TX2
    - ...
- libpcie ...
    - PCI support seems to be available  for x86, nevertheless PCIe support is currently missing
        - https://lists.sel4.systems/hyperkitty/list/devel@sel4.systems/thread/YVIO4IFLJDKXQ3YUWUWXYPHVGYLGRGME/?sort=date
    - VirtIO PCIe driver available → check difference between PCIe and VirtIO PCIe driver ?
    - ...
- libwifi, libbluetooth, ...
    - nothing similar to this currently exists, so no WiFi or BT support throughout all platforms available so far
    - ...
- libsata
    - There seems to exist something from DornerWorks, currently available as PR
        - https://github.com/seL4/util_libs/pull/82
        - https://github.com/seL4/seL4_projects_libs/pull/17
    - ...

## Errata VM support
Since Linux kernel version 5.1 if the the tegra pmc driver is not able to access registers directly it will try to use smc calls to get a higher privileged software to write to those registers. The VMM does not currently support these calls. This behaviour was introduced with commit e247deae1a550 of the LInux kernel.  It is currently (as of version 5.18) only enabled on the tegra 210 platform. We did not get the kernel to boot even if the behavior was disabled.  Typical symptom for this behaviour is this log entry 

```
[    0.112447] tegra-pmc: access to PMC is restricted to TZ
handle_smc@smc.c:68 Got SiP service call 65024
 
======= Unhandled VCPU fault from [Linux] =======
HSR Value: 0x5e000000
HSR Exception Class: HSR_SMC_64_EXCEPTION [0x17]
Instruction Length: 1
ISS Value: 0x0
==============================================
main_continued@main.c:1176 VM error -1
run@main.c:1208 VM error -1
Halting...
```



| Data61                                                     | Comment                                                                                                                    | Status  | Task                   |
|------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|---------|------------------------|
|  |  | okay |  |
