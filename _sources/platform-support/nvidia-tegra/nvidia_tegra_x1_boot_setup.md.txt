# Tegra X1 Boot Setup

## Jetson Nano 2GB Developer Kit
In order to execute TRENTOS on the Jetson Nano 2GB Developer Kit, several steps have to be done:

- Wiring of the board & connection to the PC
- One-time flashing of the TX1 QSPI and microSD card (Nvidia firmware/bootloader, CBoot, U-Boot)
- Configuration of the PXE setup
- During runtime: continuously loading a TRENTOS image from PC to the board and execute in RAM

### Wiring the board
The Jetson Nano 2GB Developer Kit requires the following physical setup:

- a USB-C power supply (5V⎓3A)
- a serial connection to a PC via a USB-to-UART adapter to the button header (J12)
    - UART2_RXD (pin 3)
    - UART2_TXD (pin 4)
    - GND (pin 7)
- in case flashing to QSPI/microSD is required
    - a USB connection to a PC via the board's USB Micro B connector (J13), required for flashing the QSPI/microSD
    - a jumper to pins 9 (GND) and 10 (FC REC) of the button header (J12), to start the board in Force Recovery Mode (RCM)
- in case PXE boot shall be used, an Ethernet cable has to be connected to the board's Ethernet interface, in order to receive relevant data from the PC

![Nvidia Tegra X1 Board](imgs/nvidia_tegra_x1_board.png)
![Nvidia Tegra X1 UART Pins](imgs/nvidia_tegra_x1_uart_pins.png)

### Boot Setup
The Jetson Nano 2GB Developer Kit regularly boots from QSPI/microSD. In general, we can therefore stick to the flashing process described in Nvidia Toolchain. As unfortunately flashing to QSPI/microSD can last up to 10 minutes, for development purposes it is recommended instead to stick to the PXE setup described in TRENTOS on Nvidia Tegra. Beware: for the PXE setup, check that the board won't be started in RCM (see respective jumper settings described above), as otherwise PXE boot will fail!

## Memory usage during the boot process
### TegraBoot handover to CBoot Entry
The memory usage when TegraBoot hands over control to CBoot.

```
0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | GSC2 Carveout                                  |
0x0'ff90'0000 +------------------------------------------------+
              | SecureOS Carveout                              |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | GSC3 Carveout                                  |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+
              | Tboot-CPU (TBC)                                |
              |     nvtboot_cpu.bin                            |
0x0'a000'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+
              | CBoot (EBT)                                    |
              |     cboot.bin                                  |
0x0'92c0'0000 +------------------------------------------------+
              | 250 MiB free                                   |
0x0'8320'0000 +------------------------------------------------+
              | U-Boot DTB (DTB)                               |
              |     kernel_tegra210-p3448-0003-p3542-0000.dtb  |
0x0'8310'0000 +------------------------------------------------+
              | CBoot DTB (RP1)                                |
              |     kernel_tegra210-p3448-0003-p3542-0000.dtb  |
0x0'8300'0000 +------------------------------------------------+   
              | 48 MiB free                                    |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

### CBoot handover to U-Boot Entry
The memory usage when CBoot hands over control to U-Boot.

```

0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   | 
0x0'????'???? +------------------------------------------------+
              | XUSB firmware (RP4)                            |
              |     rp4.blob                                   |
0x0'92ca'828c +------------------------------------------------+
              | 250,6 MiB free                                 |
0x0'8320'0000 +------------------------------------------------+
              | U-Boot DTB (DTB)                               |
              |     kernel_tegra210-p3448-0003-p3542-0000.dtb  |
0x0'8310'0000 +------------------------------------------------+
              | CBoot DTB (RP1)                                |
              |     kernel_tegra210-p3448-0003-p3542-0000.dtb  |
0x0'8300'0000 +------------------------------------------------+   
              | U-Boot (LNX)                                   |
              |     u-boot.bin                                 |
0x0'8008'0000 +------------------------------------------------+ CONFIG_SYS_TEXT_BASE in U-Boot's p3541-0000_defconfig  
              | 512 KiB free                                   |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

### U-Boot handover to Linux kernel
The memory usage when U-Boot hands over control to Linux.
```
0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+                 
              | seems U-Boot stores FTD patches here           |
              | 0x90200000 (fdtoverlay_addr_r) when            |
              | processing FDT passed from Cboot               |
0x0'9020'0000 +------------------------------------------------+ fdtoverlay_addr
              | 1 MiB reserved                                 |
0x0'9010'0000 +------------------------------------------------+ pxefile_addr_r
              | 1 MiB reserved (used in case U-Boot loads and  |
              | processes 'boot.scr'). Overwriting this memory |
              | may work for one-liners, but it's fatal for    |
              | bigger scripts.                                |
0x0'9000'0000 +------------------------------------------------+ scriptaddr
              | 192 MiB reserved                               |
              | space for uncompressed Linux kernel            |
              | ToDo: where does U-Boot put the compressed     |
              | kernel ???                                     |
0x0'8400'0000 +------------------------------------------------+ kernel_addr_r
              | 14 MiB reserved. space for Linux initrd        |
0x0'8320'0000 +------------------------------------------------+ ramdisk_addr_r
              | 2 MiB space, FTD passed from U-Boot to Linux   |
0x0'8300'0000 +------------------------------------------------+ fdt_addr_r
              | U-Boot (LNX)                                   |
              |     u-boot.bin                                 |
0x0'8008'0000 +------------------------------------------------+ CONFIG_SYS_TEXT_BASE in U-Boot's p3541-0000_defconfig    
              | 512 KiB free                                   |
              | On aarch32 U-boot passes Linux ATAG boot       |
              | param at 0x80000100                            |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

### Memory usage during TRENTOS startup
#### U-Boot handover to seL4 Elfloader

```
0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+
              | os_image.elf (via PXE)                         |
              |     image size depends on respective system    |
0x0'9000'0000 +------------------------------------------------+
              | 206 MiB free                                   |
0x0'8320'0000 +------------------------------------------------+
              | 2 MiB space, contains FTD                      |
0x0'8300'0000 +------------------------------------------------+ fdt_addr_r
              | 47,5 MiB space, used by U-boot                 |
0x0'8008'0000 +------------------------------------------------+
              | 512 KiB free                                   |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

The U-Boot 'bootelf' command processes the ELF file loaded at 0x90000000 and loads the ELF sections (there is just one code segment actually) to the requested addresses. Unfortunately, this U-Boot ELF loading does not check if one of the ELF sections overlaps with the actual ELF image and would blindly overwrite it - which then crashes the boot process. Since the seL4 Elfloader is carefully created by the seL4 build process, so its code segment's start address (here: exemplarily at 0x81b5b000) is well chosen to leave enough free space in the memory before it, the seL4 kernel and root tasks are going to be put there.

#### seL4 Elfloader handover to seL4 kernel

```
0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+
              | os_image.elf (via PXE)                         |
              |     image size depends on respective system    |
0x0'9000'0000 +------------------------------------------------+
              | 206 MiB free                                   |
0x0'831b'5117 +------------------------------------------------+
              | Elfloader, containing:                         |
              |     loader code                                |
              |     DTB                                        |
              |     seL4 kernel                                |
              |     CPIO archive with useland                  |
0x0'81b5'b000 +------------------------------------------------+ Elfloader paddr
              | 27,3 MiB free                                  |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

The seL4 Elfloader unpacks its content and hands over control to the seL4 kernel. Note that on AARCH64 the seL4 build system will configure the kernel to be located at the start of the usable RAM (here: 0x80000000). This address is taken from the device tree used during the build process (overlays can still reserve memory at the start). The location of the root task is a choice of the seL4 Elfloader. It passes this information to the kernel when handing over control.

#### seL4 kernel handover to root task
seL4 starts the root task, which is the CapDL loader that unpacks the CAmkES system and runs it.

```

0x1'0000'0000 +------------------------------------------------+ end of RAM
              | 6 MiB free                                     |
0x0'ffa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'ff80'0000 +------------------------------------------------+ PLAT_BL31_BASE in ATF's platform_t210.mk
              | 1022 MiB free                                  |
0x0'bfa0'0000 +------------------------------------------------+
              | 2 MiB system reserved                          |
0x0'bf80'0000 +------------------------------------------------+
              | ... MiB free                                   |
0x0'????'???? +------------------------------------------------+
              | kernel root task objects                       |
0x0'????'???? +------------------------------------------------+
              | free                                           |
0x0'????'???? +------------------------------------------------+
              | CAmkES system                                  | 
0x0'????'???? +------------------------------------------------+
              | free                                           | 
0x0'8175'f000 +------------------------------------------------+ 
              | capdl-loader (root task)                       | 
0x0'8025'6000 +------------------------------------------------+
              | DTB                                            |
0x0'8024'1000 +------------------------------------------------+
              | seL4 kernel                                    |
0x0'8000'0000 +------------------------------------------------+ start of RAM
```

### Bootup
When powering up the board, connect via picocom in order to receive respective boot output: from various proprietary Nvidia bootloader/firmware (e.g. from CBoot), from U-Boot and finally from TRENTOS.

```sh
$ sudo picocom -b 115200 -r -l /dev/ttyUSB0
```
You should then see something like the following output (here: exemplarily running demo_vm_minimal, loaded by U-Boot via PXE boot):

```
[0000.157] [L4T TegraBoot] (version 00.00.2018.01-l4t-8728f3cb)
[0000.162] Processing in cold boot mode Bootloader 2
[0000.167] A02 Bootrom Patch rev = 1023
[0000.171] Power-up reason: software reset
[0000.174] No Battery Present
[0000.177] pmic max77620 reset reason
[0000.180] pmic max77620 NVERC : 0x0
[0000.184] RamCode = 1
[0000.186] Platform has DDR4 type RAM
[0000.189] max77620 disabling SD1 Remote Sense
[0000.193] Setting DDR voltage to 1125mv
[0000.197] Serial Number of Pmic Max77663: 0x1707b7
[0000.205] Entering ramdump check
[0000.208] Get RamDumpCarveOut = 0x0
[0000.211] RamDumpCarveOut=0x0,  RamDumperFlag=0xe59ff3f8
[0000.217] Last reboot was clean, booting normally!
[0000.221] Sdram initialization is successful
[0000.225] SecureOs Carveout Base=0x00000000ff800000 Size=0x00800000
[0000.231] Lp0 Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.237] BpmpFw Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.243] GSC1 Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.249] Resize the SecureOs Carveout to 0x00100000
[0000.253] GSC2 Carveout Base=0x00000000ff900000 Size=0x00100000
[0000.259] GSC4 Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.265] GSC5 Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.271] GSC3 Carveout Base=0x00000000bf800000 Size=0x00200000
[0000.277] RamDump Carveout Base=0x0000000000000000 Size=0x00000000
[0000.283] Platform-DebugCarveout: 0
[0000.286] Nck Carveout Base=0x00000000ff800000 Size=0x00000000
[0000.292] Non secure mode, and RB not enabled.
[0000.296] BoardID = 3448, SKU = 0x3
[0000.299] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.303] Nano-SD: checking PT table on QSPI ...
[0000.307] NvTbootFailControlDoFailover: No failover; Continuing ...
[0000.313] Read PT from (2:0)
[0000.344] PT crc32 and magic check passed.
[0000.348] Using BFS PT to query partitions
[0000.353] Loading Tboot-CPU binary
[0000.382] Verifying TBC in OdmNonSecureSBK mode
[0000.392] Bootloader load address is 0xa0000000, entry address is 0xa0000258
[0000.399] Bootloader downloaded successfully.
[0000.403] Downloaded Tboot-CPU binary to 0xa0000258
[0000.408] MAX77620_GPIO5 configured
[0000.411] CPU power rail is up
[0000.414] CPU clock enabled
[0000.418] Performing RAM repair
[0000.421] Updating A64 Warmreset Address to 0xa00002e9
[0000.426] BoardID = 3448, SKU = 0x3
[0000.429] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.433] Nano-SD: checking PT table on QSPI ...
[0000.437] NvTbootFailControlDoFailover: No failover; Continuing ...
[0000.443] Loading NvTbootBootloaderDTB
[0000.510] Verifying NvTbootBootloaderDTB in OdmNonSecureSBK mode
[0000.579] Bootloader DTB Load Address: 0x83000000
[0000.583] BoardID = 3448, SKU = 0x3
[0000.587] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.590] Nano-SD: checking PT table on QSPI ...
[0000.595] NvTbootFailControlDoFailover: No failover; Continuing ...
[0000.601] Loading NvTbootKernelDTB
[0000.667] Verifying NvTbootKernelDTB in OdmNonSecureSBK mode
[0000.736] Kernel DTB Load Address: 0x83100000
[0000.740] BoardID = 3448, SKU = 0x3
[0000.743] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.747] Nano-SD: checking PT table on QSPI ...
[0000.751] NvTbootFailControlDoFailover: No failover; Continuing ...
[0000.759] Loading cboot binary
[0000.875] Verifying EBT in OdmNonSecureSBK mode
[0000.917] Bootloader load address is 0x92c00000, entry address is 0x92c00258
[0000.924] Bootloader downloaded successfully.
[0000.928] BoardID = 3448, SKU = 0x3
[0000.931] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.935] Nano-SD: checking PT table on QSPI ...
[0000.939] NvTbootFailControlDoFailover: No failover; Continuing ...
[0000.946] PT: Partition NCT NOT found !
[0000.949] Warning: Find Partition via PT Failed
[0000.954] Next binary entry address: 0x92c00258
[0000.958] BoardId: 3448
[0000.963] Overriding pmu board id with proc board id
[0000.967] Display board id is not available
[0000.972] No Bpmp FW loaded
[0000.974] Not loading WB0 as no bpmp/sc7entry fw
[0000.979] Set NvDecSticky Bits
[0000.982] GSC2 address ff93fffc value c0edbbcc
[0000.988] GSC MC Settings done
[0000.991] BoardID = 3448, SKU = 0x3
[0000.995] QSPI-ONLY: SkipQspiOnlyFlag = 0
[0000.998] Nano-SD: checking PT table on QSPI ...
[0001.003] NvTbootFailControlDoFailover: No failover; Continuing ...
[0001.010] TOS Image length 53680
[0001.013]  Monitor size 53680
[0001.016]  OS size 0
[0001.031] Secure Os AES-CMAC Verification Success!
[0001.035] TOS image cipher info: plaintext
[0001.039] Loading and Validation of Secure OS Successful
[0001.055] NvTbootPackSdramParams: start.
[0001.060] NvTbootPackSdramParams: done.
[0001.064] Tegraboot started after 84901 us
[0001.068] Basic modules init took 957656 us
[0001.072] Sec Bootdevice Read Time = 12 ms, Read Size = 61 KB
[0001.078] Sec Bootdevice Write Time = 0 ms, Write Size = 0 KB
[0001.083] Next stage binary read took 11287 us
[0001.087] Carveout took -69931 us
[0001.090] CPU initialization took 106106 us
[0001.094] Total time taken by TegraBoot 1005118 us
 
[0001.099] Starting CPU & Halting co-processor
 
64NOTICE:  BL31: v1.3(release):b5eeb33f7
NOTICE:  BL31: Built : 12:09:37, Jul 26 2021
ERROR:   Error initializing runtime service trusty_fast
[0001.221] RamCode = 1
[0001.225] LPDDR4 Training: Read DT: Number of tables = 2
[0001.230] EMC Training (SRC-freq: 204000; DST-freq: 1600000)
[0001.243] EMC Training Successful
[0001.246] 408000 not found in DVFS table
[0001.253] RamCode = 1
[0001.256] DT Write: emc-table@204000 succeeded
[0001.262] DT Write: emc-table@1600000 succeeded
[0001.266] LPDDR4 Training: Write DT: Number of tables = 2
[0001.305]
[0001.306] Debug Init done
[0001.309] Marked DTB cacheable
[0001.312] Bootloader DTB loaded at 0x83000000
[0001.316] Marked DTB cacheable
[0001.319] Kernel DTB loaded at 0x83100000
[0001.323] DeviceTree Init done
[0001.336] Pinmux applied successfully
[0001.341] gicd_base: 0x50041000
[0001.344] gicc_base: 0x50042000
[0001.347] Interrupts Init done
[0001.351] Using base:0x60005090 & irq:208 for tick-timer
[0001.356] Using base:0x60005098 for delay-timer
[0001.361] platform_init_timer: DONE
[0001.364] Timer(tick) Init done
[0001.368] osc freq = 38400 khz
[0001.372]
[0001.373] Welcome to L4T Cboot
[0001.376]
[0001.377] Cboot Version: 00.00.2018.01-t210-c952b4e6
[0001.382] calling constructors
[0001.385] initializing heap
[0001.388] initializing threads
[0001.391] initializing timers
[0001.393] creating bootstrap completion thread
[0001.398] top of bootstrap2()
[0001.401] CPU: ARM Cortex A57
[0001.404] CPU: MIDR: 0x411FD071, MPIDR: 0x80000000
[0001.408] initializing platform
[0001.416] Manufacturer: MF = 0xc2, ID MSB = 0x25
[0001.420] ID LSB = 0x36, ID-CFI len = 194 bytes
[0001.425] Macronix QSPI chip present
[0001.428] SPI device register
[0001.431] init boot device
[0001.434] allocating memory for boot device(SPI)
[0001.438] registering boot device
[0001.447] QSPI bdev is already initialized
[0001.451] Enable APE clock
[0001.453] Un-powergate APE partition
[0001.457] of_register: registering tegra_udc to of_hal
[0001.462] of_register: registering inv20628-driver to of_hal
[0001.468] of_register: registering ads1015-driver to of_hal
[0001.473] of_register: registering lp8557-bl-driver to of_hal
[0001.479] of_register: registering bq2419x_charger to of_hal
[0001.484] of_register: registering bq27441_fuel_gauge to of_hal
[0001.496] gpio framework initialized
[0001.499] of_register: registering tca9539_gpio to of_hal
[0001.505] of_register: registering tca9539_gpio to of_hal
[0001.510] of_register: registering i2c_bus_driver to of_hal
[0001.516] of_register: registering i2c_bus_driver to of_hal
[0001.521] of_register: registering i2c_bus_driver to of_hal
[0001.527] pmic framework initialized
[0001.530] of_register: registering max77620_pmic to of_hal
[0001.536] regulator framework initialized
[0001.540] of_register: registering tps65132_bl_driver to of_hal
[0001.546] initializing target
[0001.551] gpio_driver_register: register 'tegra_gpio_driver' driver
[0001.560] board ID = D78, board SKU = 3
[0001.563] Skipping Z3!
[0001.568] fixed regulator driver initialized
[0001.587] initializing OF layer
[0001.590] NCK carveout not present
[0001.593] Skipping dts_overrides
[0001.597] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.613] I2C Bus Init done
[0001.616] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.626] I2C Bus Init done
[0001.629] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.639] I2C Bus Init done
[0001.642] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.652] I2C Bus Init done
[0001.655] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.665] I2C Bus Init done
[0001.668] of_children_init: Ops found for compatible string maxim,max77620
[0001.678] max77620_init using irq 118
[0001.683] register 'maxim,max77620' pmic
[0001.687] gpio_driver_register: register 'max77620-gpio' driver
[0001.693] of_children_init: Ops found for compatible string nvidia,tegra210-i2c
[0001.704] I2C Bus Init done
[0001.707] NCK carveout not present
[0001.717] Find /i2c@7000c000's alias i2c0
[0001.721] get eeprom at 1-a0, size 256, type 0
[0001.730] Find /i2c@7000c500's alias i2c2
[0001.734] get eeprom at 3-a0, size 256, type 0
[0001.738] get eeprom at 3-ae, size 256, type 0
[0001.742] pm_ids_update: Updating 1,a0, size 256, type 0
[0001.748] I2C slave not started
[0001.751] I2C write failed
[0001.753] Writing offset failed
[0001.756] eeprom_init: EEPROM read failed
[0001.760] pm_ids_update: eeprom init failed
[0001.764] pm_ids_update: Updating 3,a0, size 256, type 0
[0001.795] pm_ids_update: The pm board id is 3448-0003-400
[0001.801] Adding plugin-manager/ids/3448-0003-400=/i2c@7000c500:module@0x50
[0001.810] pm_ids_update: pm id update successful
[0001.814] pm_ids_update: Updating 3,ae, size 256, type 0
[0001.844] pm_ids_update: The pm board id is 3542-0000-201
[0001.850] Adding plugin-manager/ids/3542-0000-201=/i2c@7000c500:module@0x57
[0001.858] pm_ids_update: pm id update successful
[0001.889] eeprom_get_mac: EEPROM invalid MAC address (all 0xff)
[0001.894] shim_eeprom_update_mac:267: Failed to update 0 MAC address in DTB
[0001.902] eeprom_get_mac: EEPROM invalid MAC address (all 0xff)
[0001.908] shim_eeprom_update_mac:267: Failed to update 1 MAC address in DTB
[0001.916] updating /chosen/nvidia,ethernet-mac node 48:b0:2d:2e:9c:19
[0001.923] Plugin Manager: Parse ODM data 0x000a4000
[0001.935] shim_cmdline_install: /chosen/bootargs: earlycon=uart8250,mmio32,0x70006000
[0001.949] Find /i2c@7000c000's alias i2c0
[0001.953] get eeprom at 1-a0, size 256, type 0
[0001.962] Find /i2c@7000c500's alias i2c2
[0001.966] get eeprom at 3-a0, size 256, type 0
[0001.970] get eeprom at 3-ae, size 256, type 0
[0001.975] pm_ids_update: Updating 1,a0, size 256, type 0
[0001.980] I2C slave not started
[0001.983] I2C write failed
[0001.986] Writing offset failed
[0001.989] eeprom_init: EEPROM read failed
[0001.993] pm_ids_update: eeprom init failed
[0001.997] pm_ids_update: Updating 3,a0, size 256, type 0
[0002.027] pm_ids_update: The pm board id is 3448-0003-400
[0002.033] Adding plugin-manager/ids/3448-0003-400=/i2c@7000c500:module@0x50
[0002.040] pm_ids_update: pm id update successful
[0002.045] pm_ids_update: Updating 3,ae, size 256, type 0
[0002.075] pm_ids_update: The pm board id is 3542-0000-201
[0002.081] Adding plugin-manager/ids/3542-0000-201=/i2c@7000c500:module@0x57
[0002.088] pm_ids_update: pm id update successful
[0002.118] Add serial number:1424120057998 as DT property
[0002.126] Applying platform configs
[0002.132] platform-init is not present. Skipping
[0002.137] calling apps_init()
[0002.142] Couldn't find GPT header
[0002.147] Proceeding to Cold Boot
[0002.150] starting app android_boot_app
[0002.154] Device state: unlocked
[0002.157] display console init
[0002.165] could not find regulator
[0002.189] hdmi cable not connected
[0002.192] is_hdmi_needed: HDMI not connected, returning false
[0002.198] hdmi is not connected
[0002.201] sor0 is not supportDT entry for leds-pwm not found
e[0002.209] d
[0002.211] display_console_init: no valid display out_type
[0002.219] subnode volume_up is not found !
[0002.223] subnode back is not found !
[0002.226] subnode volume_down is not found !
[0002.230] subnode menu is not found !
[0002.234] Gpio keyboard init success
[0002.314] found decompressor handler: lz4-legacy
[0002.329] decompressing blob (type 1)...
[0002.394] display_resolution: No display init
[0002.398] Failed to retrieve display resolution
[0002.403] Could not load/initialize BMP blob...ignoring
[0002.479] decompressor handler not found
[0002.482] load_firmware_blob: Firmware blob loaded, entries=2
[0002.488] XUSB blob version 0 size 126464 @ 0x92ca828c
[0002.494] -------> se_aes_verify_sbk_clear: 747
[0002.499] se_aes_verify_sbk_clear: Error
[0002.502] SE operation failed
[0002.505] bl_battery_charging: connected to external power supply
[0002.514] display_console_ioctl: No display init
[0002.519] switch_backlight failed
[0002.525] device_query_partition_size: failed to open partition spiflash0:MSC !
[0002.532] MSC Partition not found
[0002.539] device_query_partition_size: failed to open partition spiflash0:USP !
[0002.546] USP partition read failed!
[0002.549] blob_init: blob-partition USP header read failed
[0002.555] android_boot Unable to update recovery partition
[0002.560] kfs_getpartname: name = LNX
[0002.564] Loading kernel from LNX
[0002.573] Found 19 BFS partitions in "spiflash0"
[0002.898] load kernel from storage
[0002.903] decompressor handler not found
[0003.172] Successfully loaded kernel and ramdisk images
[0003.177] board ID = D78, board SKU = 3
[0003.182] sdmmc node status = okay
[0003.185] sdcard instance = 0
[0003.188] sdmmc cd-inverted
[0003.191] sdcard gpio handle 0x5a
[0003.194] sdcard gpio pin 0xc9
[0003.197] sdcard gpio flags 0x0
[0003.200] vmmc-supply 0x9a
[0003.203] cd_gpio_pin = 201
[0003.205] pin_state = 0
[0003.208] Found sdcard
[0003.210] SD-card IS present ...
[0003.213] load_and_boot_kernel: SD card detected OK
[0003.219] display_resolution: No display init
[0003.223] Failed to retrieve display resolution
[0003.228] bmp blob is not loaded and initialized
[0003.232] Failed to display boot-logo
[0003.236] NCK carveout not present
[0003.239] Skipping dts_overrides
[0003.242] NCK carveout not present
[0003.252] Find /i2c@7000c000's alias i2c0
[0003.256] get eeprom at 1-a0, size 256, type 0
[0003.265] Find /i2c@7000c500's alias i2c2
[0003.269] get eeprom at 3-a0, size 256, type 0
[0003.273] get eeprom at 3-ae, size 256, type 0
[0003.277] pm_ids_update: Updating 1,a0, size 256, type 0
[0003.283] I2C slave not started
[0003.286] I2C write failed
[0003.288] Writing offset failed
[0003.291] eeprom_init: EEPROM read failed
[0003.295] pm_ids_update: eeprom init failed
[0003.299] pm_ids_update: Updating 3,a0, size 256, type 0
[0003.330] pm_ids_update: The pm board id is 3448-0003-400
[0003.336] Adding plugin-manager/ids/3448-0003-400=/i2c@7000c500:module@0x50
[0003.345] pm_ids_update: pm id update successful
[0003.349] pm_ids_update: Updating 3,ae, size 256, type 0
[0003.379] pm_ids_update: The pm board id is 3542-0000-201
[0003.385] Adding plugin-manager/ids/3542-0000-201=/i2c@7000c500:module@0x57
[0003.393] pm_ids_update: pm id update successful
[0003.424] eeprom_get_mac: EEPROM invalid MAC address (all 0xff)
[0003.429] shim_eeprom_update_mac:267: Failed to update 0 MAC address in DTB
[0003.437] eeprom_get_mac: EEPROM invalid MAC address (all 0xff)
[0003.443] shim_eeprom_update_mac:267: Failed to update 1 MAC address in DTB
[0003.451] updating /chosen/nvidia,ethernet-mac node 48:b0:2d:2e:9c:19
[0003.458] Plugin Manager: Parse ODM data 0x000a4000
[0003.470] shim_cmdline_install: /chosen/bootargs: earlycon=uart8250,mmio32,0x70006000
[0003.478] Add serial number:1424120057998 as DT property
[0003.486] Updated bpmp info to DTB
[0003.490] Updated initrd info to DTB
[0003.494] "proc-board" doesn't exist, creating
[0003.500] Updated board info to DTB
[0003.503] "pmu-board" doesn't exist, creating
[0003.509] Updated board info to DTB
[0003.512] "display-board" doesn't exist, creating
[0003.518] Updated board info to DTB
[0003.522] "reset" doesn't exist, creating
[0003.526] Updated reset info to DTB
[0003.530] display_console_ioctl: No display init
[0003.534] display_console_ioctl: No display init
[0003.539] display_console_ioctl: No display init
[0003.543] Cmdline: tegraid=21.1.2.0.0 ddr_die=2048M@2048M section=256M memtype=0 vpr_resize usb_port_owner_info=0 lane_owner_info=0 emc_max_dvfs=0 touch_id=0@63 video=tegrafb no_console_suspend=1 console=ttyS0,115200n8 debug_uartport=lsport,4 earlyprintk=uart8250-32bit,0x70006000 maxcpus=4 usbcore.old_scheme_first=1 core_edp_mv=1125 core_edp_ma=4000 gpt
[0003.576] DTB cmdline: earlycon=uart8250,mmio32,0x70006000
[0003.581] boot image cmdline: root=/dev/mmcblk0p1 rw rootwait rootfstype=ext4 console=ttyS0,115200n8 console=tty0 fbcon=map:0 net.ifnames=0
[0003.594] Updated bootarg info to DTB
[0003.598] Adding uuid 000000016446088804000000100701c0 to DT
[0003.604] Adding eks info 0 to DT
[0003.609] WARNING: Failed to pass NS DRAM ranges to TOS, err: -7
[0003.615] Updated memory info to DTB
[0003.623] set vdd_core voltage to 1125 mv
[0003.627] setting 'vdd-core' regulator to 1125000 micro volts
[0003.633] Found secure-pmc; disable BPMP
 
 
U-Boot 2020.04-g46e4604c78 (Jul 26 2021 - 12:09:42 -0700)
 
SoC: tegra210
Model: NVIDIA Jetson Nano 2GB Developer Kit
Board: NVIDIA P3541-0000
DRAM:  2 GiB
MMC:   sdhci@700b0000: 1, sdhci@700b0600: 0
Loading Environment from SPI Flash... SF: Detected mx25u3235f with page size 256 Bytes, erase size 4 KiB, total 4 MiB
OK
In:    serial
Out:   serial
Err:   serial
Net:   No ethernet found.
Hit any key to stop autoboot:  0
Using eth_rtl8169 device
TFTP from server 10.0.0.10; our IP address is 10.0.0.11
Filename 'os_image.elf'.
Load address: 0x90000000
Loading: #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #################################################################
     #########################################################
     1.2 MiB/s
done
Bytes transferred = 23586680 (167e778 hex)
## Starting application at 0x81b5b000 ...
 
ELF-loader started on CPU: ARM Ltd. Cortex-A57 r1p1
  paddr=[81b5b000..831b5117]
No DTB passed in from boot loader.
Looking for DTB in CPIO archive...found at 81ca7f60.
Loaded DTB from 81ca7f60.
   paddr=[80241000..80255fff]
ELF-loading image 'kernel' to 80000000
  paddr=[80000000..80240fff]
  vaddr=[8080000000..8080240fff]
  virt_entry=8080000000
ELF-loading image 'capdl-loader' to 80256000
  paddr=[80256000..8175efff]
  vaddr=[400000..1908fff]
  virt_entry=409000
Enabling hypervisor MMU and paging
Jumping to kernel-image entry point...
 
Bootstrapping kernel
available phys memory regions: 2
  [80000000..bf200000]
  [c0000000..fee00000]
reserved virt address space regions: 3
  [8080000000..8080241000]
  [8080241000..80802550bd]
  [8080256000..808175f000]
Booting all finished, dropped to user space
main@main.c:2125 Starting CapDL Loader...
<<seL4(CPU 0) [decodeUntypedInvocation/209 T0x80fed31400 "rootserver" @4006f0]: Untyped Retype: Insufficient memory (1 * 16777216 bytes needed, 0 bytes available).>>
<<seL4(CPU 0) [decodeUntypedInvocation/209 T0x80fed31400 "rootserver" @4006f0]: Untyped Retype: Insufficient memory (1 * 2097152 bytes needed, 0 bytes available).>>
main@main.c:2127 CapDL Loader done, suspending...
main_continued@main.c:1156 vmm_pci_init()
vmm_pci_add_entry@pci.c:67 Adding virtual PCI device at 00:00.0
main_continued@main.c:1171 VMM init
main_continued@main.c:1176 initialize VM 'vm0'
main_continued@main.c:1179 VM0 ('vm0_irq_thread_entry@irq_server.c:130 thread started. Waiting on endpoint 107
 
')
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x50046000
load_linux@main.c:850 Loading Linux:
  kernel  'linux'
  dtb:    'linux-dtb'
  initrd: 'linux-initrd'
load_linux@main.c:854 installing linux devices...
install_linux_devices@main.c:656 module: map_frame_hack
install_linux_devices@main.c:656 module: init_ram
init_ram_module@init_ram.c:23 register RAM: 0xc0000000 - 0xd0000000
Loading Kernel: 'linux'
load_linux@main.c:874 loaded into VM: 0xc0080000 - 0xc10c1808 (kernel 'linux')
load_linux@main.c:890 loaded into VM: 0xcd700000 - 0xce741808 (initrd 'linux-initrd')
load_linux@main.c:895 don't load DTB file, CONFIG_VM_DTB_FILE not set
generate_fdt@main.c:726 Generating FTD...
fdtgen_keep_node_subtree@fdtgen.c:447 Non-existing root node /clocks
fdtgen_keep_node_subtree@fdtgen.c:447 Non-existing root node /regulators
clean_up@fdtgen.c:374 Non-existing node /interrupt-controller@50041000 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /pmu@7000a000 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /sdhci@700b0000 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /sdhci@700b0600 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /i2c@7000d400 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /i2c@7000d600 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /i2c@7000d800 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /i2c@7000da00 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /sdhci@700b0200 specified to be kept
clean_up@fdtgen.c:374 Non-existing node /sdhci@700b0400 specified to be kept
load_linux@main.c:936 loaded into VM: 0xcf000000 - 0xcf050000 (generated DTB)
Loading Generated DTB
main_continued@main.c:1255 run VM0 ('vm0')
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x70006004, memory fault left unhandled
OnDemandInstall: Created device-backed memory for addr 0x70006000
[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x411fd071]
[    0.000000] Linux version 4.16.0 (alisonf@shinyu-un) (gcc version 6.3.0 20170516 (Debian 6.3.0-18)) #7 SMP Tue Nov 19 18:36:54 AEDT 2019
[    0.000000] Machine model: NVIDIA Jetson Nano Developer Kit
[    0.000000] debug: ignoring loglevel setting.
[    0.000000] earlycon: uart8250 at MMIO32 0x0000000070006000 (options '')
[    0.000000] bootconsole [uart8250] enabled
[    0.000000] On node 0 totalpages: 65536
[    0.000000]   DMA32 zone: 1024 pages used for memmap
[    0.000000]   DMA32 zone: 0 pages reserved
[    0.000000]   DMA32 zone: 65536 pages, LIFO batch:15
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.0 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: Trusted OS migration not required
[    0.000000] psci: SMC Calling Convention v1.0
[    0.000000] random: fast init done
[    0.000000] percpu: Embedded 23 pages/cpu @        (ptrval) s54472 r8192 d31544 u94208
[    0.000000] pcpu-alloc: s54472 r8192 d31544 u94208 alloc=23*4096
[    0.000000] pcpu-alloc: [0] 0
[    0.000000] Detected PIPT I-cache on CPU0
[    0.000000] CPU features: enabling workaround for ARM erratum 832075
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 64512
[    0.000000] Kernel command line: nosmp debug ignore_loglevel console=ttyS0,115200n8 earlycon=uart8250,mmio32,0x70006000 maxcpus=1
[    0.000000] Dentry cache hash table entries: 32768 (order: 6, 262144 bytes)
[    0.000000] Inode-cache hash table entries: 16384 (order: 5, 131072 bytes)
[    0.000000] Memory: 214832K/262144K available (9726K kernel code, 774K rwdata, 3388K rodata, 2048K init, 12438K bss, 47312K reserved, 0K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=1, Nodes=1
[    0.000000] Running RCU self tests
[    0.000000] Hierarchical RCU implementation.
[    0.000000]  RCU event tracing is enabled.
[    0.000000]  RCU lockdep checking is enabled.
[    0.000000]  RCU restricting CPUs from NR_CPUS=4 to nr_cpu_ids=1.
[    0.000000] RCU: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=1
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x60004028, memory fault left unhandled
OnDemandInstall: Created device-backed memory for addr 0x60004000
[    0.000000] /interrupt-controller@60004000: 192 interrupts forwarded to /interrupt-controller
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x6000655c, memory fault left unhandled
OnDemandInstall: Created device-backed memory for addr 0x60006000
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x7000e4f8, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x7000e000
OnDemandInstall: Created RAM-backed memory for addr 0x7000e000
[    0.000000] PLL_RE already enabled. Postponing set full defaults
[    0.000000] arch_timer: cp15 timer(s) running at 19.20MHz (virt).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x46d987e47, max_idle_ns: 440795202767 ns
[    0.000007] sched_clock: 56 bits at 19MHz, resolution 52ns, wraps every 4398046511078ns
[    0.008606] Console: colour dummy device 80x25
[    0.013210] Lock dependency validator: Copyright (c) 2006 Red Hat, Inc., Ingo Molnar
[    0.021221] ... MAX_LOCKDEP_SUBCLASSES:  8
[    0.025445] ... MAX_LOCK_DEPTH:          48
[    0.029755] ... MAX_LOCKDEP_KEYS:        8191
[    0.034271] ... CLASSHASH_SIZE:          4096
[    0.038759] ... MAX_LOCKDEP_ENTRIES:     32768
[    0.043338] ... MAX_LOCKDEP_CHAINS:      65536
[    0.047953] ... CHAINHASH_SIZE:          32768
[    0.052530]  memory used by lock dependency info: 7391 kB
[    0.058130]  per task-struct memory footprint: 1920 bytes
[    0.063720] Calibrating delay loop (skipped), value calculated using timer frequency.. 38.40 BogoMIPS (lpj=192000)
[    0.074426] pid_max: default: 32768 minimum: 301
[    0.079361] Mount-cache hash table entries: 512 (order: 0, 4096 bytes)
[    0.086130] Mountpoint-cache hash table entries: 512 (order: 0, 4096 bytes)
[    0.095888] ASID allocator initialised with 32768 entries
[    0.101791] Hierarchical SRCU implementation.
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x70000804, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x70000000
OnDemandInstall: Created RAM-backed memory for addr 0x70000000
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x7000f910, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x7000f000
OnDemandInstall: Created RAM-backed memory for addr 0x7000f000
[    0.157286] Speedo Revision 0
[    0.162259] ------------[ cut here ]------------
[    0.167015] speedo value not fused
[    0.170619] WARNING: CPU: 0 PID: 1 at tegra210_init_speedo_data+0x154/0x220
[    0.177803] Modules linked in:
[    0.180964] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 4.16.0 #7
[    0.187072] Hardware name: NVIDIA Jetson Nano Developer Kit (DT)
[    0.193265] pstate: 60000005 (nZCv daif -PAN -UAO)
[    0.198206] pc : tegra210_init_speedo_data+0x154/0x220
[    0.203508] lr : tegra210_init_speedo_data+0x154/0x220
[    0.208804] sp : ffffff8008023c80
[    0.212219] x29: ffffff8008023c80 x28: 0000000000000000
[    0.217704] x27: 0000000000000000 x26: 0000000000000000
[    0.223196] x25: 0000000000000000 x24: 0000000000000000
[    0.228679] x23: 0000000000000000 x22: 0000000000000000
[    0.234163] x21: 0000000000000000 x20: 00000000ffffffb5
[    0.239647] x19: ffffff8009cb1038 x18: ffffffffffffffff
[    0.245131] x17: 00000000d4cd6119 x16: 0000000000000000
[    0.250618] x15: ffffff800900ea08 x14: ffffff8089c999ef
[    0.256102] x13: ffffff8009c999fd x12: ffffff8009021710
[    0.261590] x11: ffffffc00e4687a0 x10: ffffff8009037000
[    0.267074] x9 : ffffff8009c9c000 x8 : ffffffc00e468000
[    0.272553] x7 : ffffff80080f21d4 x6 : 00000000bda4bdf1
[    0.278041] x5 : 0000000000000100 x4 : 0000000000000000
[    0.283522] x3 : 0000000000000000 x2 : ffffff8009c9c000
[    0.289012] x1 : 24c07bbf160ed200 x0 : 0000000000000000
[    0.294493] Call trace:
[    0.297014]  tegra210_init_speedo_data+0x154/0x220
[    0.301951]  tegra30_fuse_init+0x5c/0x114
[    0.306088]  tegra_init_fuse+0x178/0x1c8
[    0.310139]  do_one_initcall+0x50/0x160
[    0.314099]  kernel_init_freeable+0x78/0x1d0
[    0.318508]  kernel_init+0x10/0x100
[    0.322104]  ret_from_fork+0x10/0x18
[    0.325795] ---[ end trace e6e6b509e7920e75 ]---
[    0.330632] Tegra Revision: unknown SKU: 0 CPU Process: 0 SoC Process: 0
[    0.338560] OF: /pmc@7000e400/powergates/venc: could not find phandle
[    0.345276] WARNING: CPU: 0 PID: 1 at __alloc_pages_nodemask+0x1b0/0xa88
[    0.352179] Modules linked in:
[    0.355339] CPU: 0 PID: 1 Comm: swapper/0 Tainted: G        W        4.16.0 #7
[    0.362785] Hardware name: NVIDIA Jetson Nano Developer Kit (DT)
[    0.368977] pstate: 20000005 (nzCv daif -PAN -UAO)
[    0.373918] pc : __alloc_pages_nodemask+0x1b0/0xa88
[    0.378950] lr : __alloc_pages_nodemask+0xdc/0xa88
[    0.383885] sp : ffffff8008023b80
[    0.387300] x29: ffffff8008023b80 x28: 0000000000000008
[    0.392783] x27: 0000000000000001 x26: ffffffc00e4b0b00
[    0.398269] x25: ffffff80090b2000 x24: 0000000000000000
[    0.403751] x23: ffffffc00ffba508 x22: 0000000000000017
[    0.409238] x21: 0000000000000000 x20: ffffff800900e000
[    0.414725] x19: 000000000140c0c0 x18: ffffffffffffffff
[    0.420207] x17: ffffff8009979328 x16: 0000000000000001
[    0.425691] x15: ffffff800900ea08 x14: ffffff8089c999ef
[    0.431172] x13: ffffff8009c999fd x12: ffffff8009021710
[    0.436655] x11: ffffffc00e4687a0 x10: ffffff8009037000
[    0.442141] x9 : ffffff8009c9c000 x8 : ffffffc00e468000
[    0.447622] x7 : 0000000000000000 x6 : 00000000007fffff
[    0.453103] x5 : ffffffffff80d0fa x4 : 0000000000000000
[    0.458587] x3 : 0000000000000000 x2 : 24c07bbf160ed200
[    0.464070] x1 : 0000000000000000 x0 : 0000000000000000
[    0.469552] Call trace:
[    0.472072]  __alloc_pages_nodemask+0x1b0/0xa88
[    0.476748]  kmalloc_order+0x1c/0x38
[    0.480439]  __kmalloc+0x11c/0x170
[    0.483945]  tegra_pmc_early_init+0x3b4/0x5a0
[    0.488436]  do_one_initcall+0x50/0x160
[    0.492390]  kernel_init_freeable+0x78/0x1d0
[    0.496791]  kernel_init+0x10/0x100
[    0.500386]  ret_from_fork+0x10/0x18
[    0.504068] ---[ end trace e6e6b509e7920e76 ]---
[    0.508870] tegra-pmc: failed to get resets for venc: -12
[    0.514813] smp: Bringing up secondary CPUs ...
[    0.519524] smp: Brought up 1 node, 1 CPU
[    0.523657] SMP: Total of 1 processors activated.
[    0.528515] CPU features: detected feature: 32-bit EL0 Support
[    0.534568] CPU features: detected feature: Kernel page table isolation (KPTI)
[    0.545895] CPU: All CPU(s) started at EL1
[    0.550139] alternatives: patching kernel code
[    0.556996] devtmpfs: initialized
[    0.584634] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604462750000 ns
[    0.594943] futex hash table entries: 256 (order: 3, 32768 bytes)
[    0.601500] pinctrl core: initialized pinctrl subsystem
[    0.609147] NET: Registered protocol family 16
[    0.615203] cpuidle: using governor menu
[    0.619480] vdso: 2 pages (1 code @ 00000000c776c61b, 1 data @ 000000006ac73153)
[    0.627110] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.634402] DMA: preallocated 256 KiB pool for atomic allocations
[    0.666100] vgaarb: loaded
[    0.669485] SCSI subsystem initialized
[    0.673650] libata version 3.00 loaded.
[    0.678117] usbcore: registered new interface driver usbfs
[    0.683855] usbcore: registered new interface driver hub
[    0.689599] usbcore: registered new device driver usb
[    0.695288] tegra-i2c 7000c700.i2c: could not find pctldev for node /host1x@50000000/dpaux@54040000/pinmux-i2c, deferring probe
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x7000d000, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x7000d000
OnDemandInstall: Created RAM-backed memory for addr 0x7000d000
[    1.753969] tegra-i2c 7000d000.i2c: timeout waiting for fifo flush
[    1.760384] tegra-i2c 7000d000.i2c: Failed to initialize i2c controller
[    1.767334] tegra-i2c: probe of 7000d000.i2c failed with error -110
[    1.773876] media: Linux media interface: v0.10
[    1.778647] Linux video capture interface: v2.00
[    1.783525] pps_core: LinuxPPS API ver. 1 registered
[    1.788682] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    1.798127] PTP clock support registered
[    1.802634] Advanced Linux Sound Architecture Driver Initialized.
[    1.810015] Bluetooth: Core ver 2.22
[    1.813817] NET: Registered protocol family 31
[    1.818400] Bluetooth: HCI device and connection manager initialized
[    1.825030] Bluetooth: HCI socket layer initialized
[    1.830067] Bluetooth: L2CAP socket layer initialized
[    1.835393] Bluetooth: SCO socket layer initialized
[    1.841330] clocksource: Switched to clocksource arch_sys_counter
[    1.848190] VFS: Disk quotas dquot_6.6.0
[    1.852396] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    1.872016] OF: /thermal-zones/cpu-thermal/cooling-maps/cpu-critical: could not find phandle
[    1.880724] missing cooling_device property
[    1.885095] failed to build thermal zone cpu-thermal: -22
[    1.890762] OF: /thermal-zones/mem-thermal/cooling-maps/dram-passive: could not find phandle
[    1.899501] missing cooling_device property
[    1.903854] failed to build thermal zone mem-thermal: -22
[    1.910176] NET: Registered protocol family 2
[    1.915805] tcp_listen_portaddr_hash hash table entries: 128 (order: 1, 9216 bytes)
[    1.923867] TCP established hash table entries: 2048 (order: 2, 16384 bytes)
[    1.931156] TCP bind hash table entries: 2048 (order: 5, 131072 bytes)
[    1.938446] TCP: Hash tables configured (established 2048 bind 2048)
[    1.945347] UDP hash table entries: 256 (order: 3, 40960 bytes)
[    1.951691] UDP-Lite hash table entries: 256 (order: 3, 40960 bytes)
[    1.958807] NET: Registered protocol family 1
[    1.964409] RPC: Registered named UNIX socket transport module.
[    1.970529] RPC: Registered udp transport module.
[    1.975439] RPC: Registered tcp transport module.
[    1.980288] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    1.987705] PCI: CLS 0 bytes, default 128
[    2.066755] Trying to unpack rootfs image as initramfs...
[    2.154150] Freeing initrd memory: 888K
[    2.160314] Initialise system trusted keyrings
[    2.165519] workingset: timestamp_bits=62 max_order=16 bucket_order=0
[    2.182918] NFS: Registering the id_resolver key type
[    2.188180] Key type id_resolver registered
[    2.192602] Key type id_legacy registered
[    2.196815] jffs2: version 2.2. (NAND) © 2001-2006 Red Hat, Inc.
[    2.203968] fuse init (API version 7.26)
[    2.213938] Key type asymmetric registered
[    2.218222] Asymmetric key parser 'x509' registered
[    2.223467] io scheduler noop registered
[    2.227518] io scheduler deadline registered
[    2.232408] io scheduler cfq registered (default)
[    2.237262] io scheduler mq-deadline registered
[    2.242018] io scheduler kyber registered
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x70003000, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x70003000
OnDemandInstall: Created RAM-backed memory for addr 0x70003000
vm_memory_handle_fault@guest_memory.c:274 Unable to find reservation for addr: 0x60020000, memory fault left unhandled
_utspace_split_alloc@split.c:266 Failed to find any untyped capable of creating an object at address 0x60020000
OnDemandInstall: Created RAM-backed memory for addr 0x60020000
[    2.305346] tegra-apbdma 60020000.dma: Tegra20 APB DMA driver register 32 channels
[    2.314585] tegra-pmc 7000e400.pmc: i2c-thermtrip node not found, emergency thermal reset disabled.
[    2.324605] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
[    2.335804] console [ttyS0] disabled
[    2.339790] 70006000.serial: ttyS0 at MMIO 0x70006000 (irq = 55, base_baud = 25500000) is a Tegra
[    2.349231] console [ttyS0] enabled
[    2.349231] console [ttyS0] enabled
[    2.356371] bootconsole [uart8250] disabled
[    2.356371] bootconsole [uart8250] disabled
[    2.367221] cacheinfo: Unable to detect cache hierarchy for CPU 0
[    2.385726] brd: module loaded
[    2.403080] loop: module loaded
[    2.408167] libphy: Fixed MDIO Bus: probed
[    2.413057] CAN device driver interface
[    2.417257] usbcore: registered new interface driver asix
[    2.422761] usbcore: registered new interface driver ax88179_178a
[    2.428891] usbcore: registered new interface driver cdc_ether
[    2.434806] usbcore: registered new interface driver net1080
[    2.440501] usbcore: registered new interface driver cdc_subset
[    2.446504] usbcore: registered new interface driver zaurus
[    2.452188] usbcore: registered new interface driver cdc_ncm
[    2.457845] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
[    2.464405] ehci-pci: EHCI PCI platform driver
[    2.468969] usbcore: registered new interface driver usb-storage
[    2.476993] i2c /dev entries driver
[    2.481594] Bluetooth: HCI UART driver ver 2.3
[    2.486044] Bluetooth: HCI UART protocol H4 registered
[    2.491540] Bluetooth: HCI UART protocol LL registered
[    2.497138] sdhci: Secure Digital Host Controller Interface driver
[    2.503365] sdhci: Copyright(c) Pierre Ossman
[    2.507722] sdhci-pltfm: SDHCI platform and OF driver helper
[    2.514164] usbcore: registered new interface driver usbhid
[    2.519731] usbhid: USB HID core driver
[    2.527753] NET: Registered protocol family 10
[    2.534372] Segment Routing with IPv6
[    2.538164] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    2.545185] NET: Registered protocol family 17
[    2.549650] can: controller area network core (rev 20170425 abi 9)
[    2.556050] NET: Registered protocol family 29
[    2.560500] can: raw protocol (rev 20170425)
[    2.564884] can: broadcast manager protocol (rev 20170425 t)
[    2.570551] can: netlink gateway (rev 20170425) max_hops=1
[    2.576411] Key type dns_resolver registered
[    2.581769] Loading compiled-in X.509 certificates
[    2.623471] tegra-i2c 7000c700.i2c: could not find pctldev for node /host1x@50000000/dpaux@54040000/pinmux-i2c, deferring probe
[    2.637837] OF: /gpio-keys/power: could not find phandle
[    2.643269] gpio-keys gpio-keys: failed to get gpio: -22
[    2.648773] gpio-keys: probe of gpio-keys failed with error -22
[    2.654813] hctosys: unable to open rtc device (rtc0)
[    2.660092] cfg80211: Loading compiled-in X.509 certificates for regulatory database
[    2.675469] cfg80211: Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
[    2.683172] ALSA device list:
[    2.686146]   No soundcards found.
[    2.690056] platform regulatory.0: Direct firmware load for regulatory.db failed with error -2
[    2.698843] cfg80211: failed to load regulatory.db
[    2.706234] Freeing unused kernel memory: 2048K
irq_handler@main.c:630 IRQ 68 Dropped
Starting syslogd: OK
Starting klogd: OK
Running sysctl: OK
Initializing random number generator... done.
Starting network: OK
 
Welcome to Buildroot
buildroot login:
```


