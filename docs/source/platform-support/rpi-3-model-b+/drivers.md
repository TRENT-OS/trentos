# RPi3B+ Drivers

TRENTOS includes drivers for accessing the peripheral devices of the
Raspberry Pi 3 Model B+ (RPi3B+).

## Timer

The timer driver of the RPi3B+ is implemented in the seL4 libplatsupport
library. Timer functionalities in TRENTOS are provided through
the [TimeServer](../../components/time-server.md) component.

## Ethernet

The Raspberry Pi uses an onboard USB-based Ethernet device. The USB (and
Ethernet) support on the RPi3B+ is based on a chain of different HW IPs,
tightly connected together.

The ethernet driver is based on the project USPi found
in <https://github.com/rsta2/uspi> licensed under GNU General Public
License v3.0.

Within TRENTOS the ethernet driver functionality is encapsulated in the
[NIC_RPi](../../components/nic_rpi.md) component.

## GPIO

Support for accessing and manipulating GPIOs on the RPi3B+ is provided
by seL4 libplatsupport.

A user can use the following includes to access GPIO related functions.

```c
#include <platsupport/gpio.h>
#include <platsupport/plat/gpio.h>
```

As an example, GPIO support is used in the
[SdHostController](../../components/sd-host-controller.md) component for the
RPi3B+.

## Mailbox

Support for writing to and reading from the RPi3B+ mailbox is provided
by seL4 libplatsupport.

A user can use the following includes to access mailbox-related
functions.

```c
#include <platsupport/mach/mailbox_util.h>
```

As an example, mailbox support is used in the [NIC_RPi](../../components/nic_rpi.md)
and [SdHostController](../../components/sd-host-controller.md) component for the
RPi3B+.

## SD Host Controller

The host controller functionalities for interfacing with an SD card in
TRENTOS are provided through the
[SdHostController](../../components/sd-host-controller.md) component that
implements the generic TRENTOS CAmkES storage interface.

On the RPi3B+, the ports of the host controller are mapped as follows:

| Port                               | MMC          |
|------------------------------------|--------------|
| SDHC1<br>(Default Port on TRENTOS) | microSD Slot |

## SPI / Flash

To enable flash access via SPI on the RPi3B+ TRENTOS provides
the [RPi_SPI_Flash](../../components/rpi_spi_flash.md) component.

It encapsulates drivers for the SPI found onboard the RPi3B+ together
with drivers for flash connected via SPI.

The included SPI driver is based
on: <https://www.airspayce.com/mikem/bcm2835/> under GNU General Public
License v3.0.

The included SPI flash driver is based
on: <https://github.com/pellepl/spiflash_driver> under MIT License.
