# Nitrogen6_SoloX Drivers

The TRENTOS platform support includes drivers for accessing the peripheral
devices of the Nitrogen6_SoloX.

## Timer

The timer driver of the board is implemented as part of the i.MX6
support in the seL4 libplatsupport library. Timer functionalities in
TRENTOS are provided through the [TimeServer](../../components/time-server.md)
component.

## Ethernet

The ethernet driver for the Nitrogen6_SoloX is also implemented as part
of the i.MX6 support in the seL4 libplatsupport library included in the
SDK. The driver functionality is encapsulated in the
[NIC_iMX6](../../components/nic_imx6.md) component that also adds the additional
feature of Dual NIC support. This allows making full usage of the two ethernet
ports present on the Nitrogen6_SoloX.

## SD Host Controller

The host controller functionalities for interfacing with an SD card in
TRENTOS are provided through the
[SdHostController](../../components/sd-host-controller.md) component that
implements the generic TRENTOS CAmkES storage interface.

**Warning:** Please be aware that the current state of the driver does
not yet provide the required functionality to read out the card
detection PIN on the Nitrogen6_SoloX platform. Using the function that
the storage interface provides to check whether or not an SD card is
currently present is therefore set to always return positive.

On the Nitrogen6_SoloX, the ports of the host controller are mapped as
follows:

| Port                               | MMC                     |
|------------------------------------|-------------------------|
| SDHC2<br>(Default Port on TRENTOS) | microSD Slot            |
| SDHC4                              | eMMC<br>(not supported) |
