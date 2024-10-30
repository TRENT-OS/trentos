# Nitrogen6_SoloX Debugging with JTAG

In the following, an exemplary setup is presented which allows JTAG debugging on
the Nitrogen6_SoloX board using an Adafruit FT232H board. Please refer to the
[Runtime Debugging](../../development/runtime-debugging/runtime-debugging.md)
section to get an overview on debugging TRENTOS systems and the required tools.
This chapter only provides the debugging details specific to the
Nitrogen6_SoloX platform.

## Nit6X_JTAG Adapter

For JTAG, the boards uses a 10 pin (2x5, 1.27 mm pitch) connector.
BoundaryDevices offers the "Nit6X_JTAG" adapter
(<https://boundarydevices.com/product/nit6x_jtag/>), which provide the common 20
pin (2x10, 2.54 mm pitch) connector,that can be easily be connected to the
Adafruit FT232H board.

## JTAG Pin Mapping

In order to properly connect the Nit6X_JTAG to the Adafruit FT232H, connect the
following pins used for JTAG communication:

| JTAG Signal            | Adafruit FT232H | Cable Color | Nit6X_JTAG Pin |
|------------------------|-----------------|-------------|----------------|
| SRST (System Reset)    | -               | -           | -              |
| TRST (Test Reset)      | C0              | white       | 3              |
| TCK (Test Clock)       | D0              | brown       | 9              |
| TMS (Test Mode Select) | D3              | violet      | 7              |
| TDI (Test Data In)     | D1              | blue        | 5              |
| TDO (Test Data Out)    | D2              | green       | 13             |
| GND                    | GND             | black       | 4              |

!["Nitrogen6_SoloX - HW Setup JTAG"](img/debugging_jtag.png)

## OpenOCD Configuration File

As mentioned in the
[Debugging HW Platforms Using OpenOCD](../../development/runtime-debugging/debugging-hw-platforms_open-ocd.md)
section, OpenOCD requires two different configuration files - one for the
FT232H board and one regarding the particular platform. The configuration file
specific to the Nitrogen6_SoloX platform is located in the same directory as
the FT232H configuration file (**`sdk/resources/openocd_cfgs/nitrogen6sx.cfg`**).

## Segger Toolchain Setup

The HW setup of the Nitrogen6_SoloX platform for use with the Segger
J-Link Base Compact is done as follows:

1. Connect the J-Link Adapter Cortex-M
    (<https://www.segger.com/products/debug-probes/j-link/accessories/adapters/9-pin-cortex-m-adapter/>)
    to the J-Link Base Compact (simple plug-in)
2. Connect the 10-pin connector from the Adapter Cortex-M adapter to
    the Nitrogen6_SoloX using the 10-pin cable (red cable marks the
    pin 1)

The HW setup is shown in the image below:

!["Nitrogen6_SoloX - HW Setup Segger"](img/debugging_setup-segger.jpg)

In addition to the HW setup, setting up the Nitrogen6_SoloX platform
for use with the Segger toolchain requires additional steps compared to
the [Debugging HW Platforms Using Segger Tools](../../development/runtime-debugging/debugging-hw-platforms_segger.md)
section. Specifically, the difference stems from the fact that the
Nitrogen6_SoloX has two ARM cores - the Cortex-A9 and the Cortex-M4. In
order to initialize both cores correctly it is necessary to perform the
steps described here:
[https://wiki.segger.com/i.MX6SoloX.](https://wiki.segger.com/i.MX6SoloX)
In short, a special J-Link script needs to be added in the last step of
the Ozone project configuration. Since we are using the Cortex-A9 core
of the Nitrogen6_SoloX board, the script available here:
<https://wiki.segger.com/File:iMX6SoloX_Connect_CortexA9.JLinkScript>
needs to be downloaded and selected as shown in the image below:

!["Nitrogen6_SoloX - Last Step of the Ozone Setup"](img/debugging_ozone.png)
