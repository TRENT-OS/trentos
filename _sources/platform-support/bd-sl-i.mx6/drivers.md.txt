# BD-SL-i.MX6 Drivers

The TRENTOS platform support includes drivers for accessing the peripheral
devices of the BD-SL-i.MX6.

## Timer

The timer driver of the board is implemented as part of the i.MX6 support in the
seL4 libplatsupport library. Timer functionalities in TRENTOS are provided
through the [TimeServer](../../components/time-server.md) component.

## Ethernet

The ethernet driver for the BD-SL-i.MX6 is also implemented as part of
the i.MX6 support in the seL4 libplatsupport library included in the
SDK. The driver functionality is encapsulated in the
[NIC_iMX6](../../components/nic_imx6.md) component and the specific possible
configuration details can be found in the documentation of this component.

## SD Host Controller

The host controller functionalities for interfacing with an SD card in
TRENTOS are provided through the
[SdHostController](../../components/sd-host-controller.md) component that
implements the generic TRENTOS CAmkES storage interface.

On the BD-SL-i.MX6, the ports of the host controller are mapped as
follows:

| Port                               | MMC              |
|------------------------------------|------------------|
| SDHC3                              | Standard SD slot |
| SDHC4<br>(Default Port on TRENTOS) | micro SD slot    |
