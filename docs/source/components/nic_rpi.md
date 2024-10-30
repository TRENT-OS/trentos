# NIC_RPi

## Overview

The NIC_RPi driver allows using the network interface on the Raspberry
Pi (RPi) version 3 B+. It exposes the **`if_OS_Nic`** interface.

### Hardware

The RPi uses an onboard USB-based Ethernet device. The USB (and
Ethernet) support on the RPi is based on a chain of different, tightly
connected HW IPs:

- A USB 2.0 Controller (Synopsys DesignWare HS OTG,
    <https://www.synopsys.com/dw/ipdir.php?ds=dwc_usb_2_0_hs_otg>)
    integrated in the BCM2837 SoC.
- A USB 2.0 Hub with up to four USB downstream ports and a 10/100/1000
    Ethernet Controller (depending on the respective RPi version).
  - RPi3B: LAN9514
        (<https://www.microchip.com/wwwproducts/en/LAN9514>) allowing
        max. 94,1 MBit/s (up) and max. 95,5 MBit/s (down) throughput.
  - RPi3B+: LAN7515, which is a USB 2.0 version of LAN7800
        (<https://www.microchip.com/wwwproducts/en/LAN7800>), allowing
        max. 315 MBit/s up/down throughput.

### Implementation

The NIC_RPi driver consists of two parts:

- A library that offers functionality to deal with the
  - low-level Broadcom VideoCore GPU configuration and startup (e.g.
        Mailbox hardware)
  - initialization of the USB hardware
  - initialization of the Ethernet hardware on top of USB
- A TRENTOS specific wrapper driver that uses the library internally
    and
  - adapts the library to TRENTOS specific features (e.g. memory
        allocation, error handling)
  - facilitates communication between the VideoCore GPU and the ARM
        CPU via the Mailbox hardware
  - provides network capabilities to a network stack running on top

The library supports both RPi3B and RPi3B+, but the TRENTOS specific
wrapper driver has only been tested with the RPi3B+.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The NIC_RPi driver can be declared via a simple macro in the
**`CMakeLists.txt`** file:

```CMake
NIC_RPi_DeclareCAmkESComponent(
    <NameOfNICComponent>
)
```

### Instantiation and Configuration in CAmkES

In order to wire the NIC_RPi driver to the system, it has to be
declared, instantiated, connected and configured in the main CAmkES
composition of the system.

#### Declaring the Component

The NIC_RPi driver consists of three parts, that have to be declared
separately:

- The Mailbox hardware device
- The USB hardware device
- The network driver

See below for the correct declaration:

```c
#include "NIC_RPi/NIC_RPi.camkes"
NIC_RPi_COMPONENT_DEFINE(<NameOfNICComponent>, NIC_DRIVER_RINGBUFFER_SIZE)
NIC_RPi_Mailbox_COMPONENT_DEFINE(<NameOfMailboxComponent>)
NIC_RPi_USB_COMPONENT_DEFINE(<NameOfUSBComponent>)
```

Please note that the name of the driver component must match the name
used in the CMake file.

#### Instantiating and Connecting the Component

The following macro connects all instances of all hardware components to
an instance of the main driver:

```c
component <NameOfNICComponent>       <nameOfNICInstance>;
component <NameOfMailboxComponent>   <nameOfMailboxInstance>;
component <NameOfUSBComponent>       <nameOfUSBInstance>;

NIC_RPi_INSTANCE_CONNECT(
    <nameOfNICInstance>,
    <nameOfMailboxInstance>,
    <nameOfUSBInstance>
)
```

A network stack running on top of the network driver has to use the
**`if_OS_Nic`** interface and provide an input/output buffer. It also
has to connect to an event that signals the reception of new data by the
network driver. This can be done either by making use of the macros
provided by the **`if_OS_Nic`** interface (see the
[if_OS_Nic](../component-interfaces/nic-interface.md) interface documentation
for more information) or by using the respective macros provided by the
component the driver should be connected to (see for example the documentation
of the [NetworkStack_PicoTcp](network-stack_pico-tcp.md) component).

#### Configuring the Instance

The NIC_RPi driver has to be configured accordingly before usage within
a TRENTOS system environment.

As the driver is made up of three components, each of them has to be
configured separately. For Mailbox and USB, the driver can configure
itself, just the respective macros have to be invoked.  As the driver
internally uses DMA, a certain amount of memory has to be provided
upfront.

The full macros look like this:

```c
NIC_RPi_Mailbox_INSTANCE_CONFIGURE_SELF(
    <nameOfMailboxInstance>
)
NIC_RPi_USB_INSTANCE_CONFIGURE_SELF(
    <nameOfUSBInstance>
)
NIC_RPi_INSTANCE_CONFIGURE(
    <nameOfNICInstance>,
    <DMASize>
)
```

## Example

Here we see how to use the NIC_RPi driver. Usually, it would be connected to the
[Socket API](../api/socket_api.md) through a network stack component.

### Instantiation of the Component in CMake

The NIC_RPi driver can simply be added to the build like this:

```CMake
NIC_RPi_DeclareCAmkESComponent(
    NIC_RPi
)
```

### Instantiation and Configuration in CAmkES

In this example, the component is connected to a second component which
acts as network stack.

#### Declaring the Component

The NIC_RPi driver can be declared as follows:

```c
#include "NIC_RPi/NIC_RPi.camkes"
NIC_RPi_COMPONENT_DEFINE(NIC_RPi, NIC_DRIVER_RINGBUFFER_SIZE)
NIC_RPi_Mailbox_COMPONENT_DEFINE(NIC_RPi_Mailbox)
NIC_RPi_USB_COMPONENT_DEFINE(NIC_RPi_USB)
```

#### Instantiating and Connecting the Component

To connect the NIC_RPi driver with a network stack component, the
following code can be used:

```c
// Instantiate the driver component
component NIC_RPi           nwDriver;
// Instantiate the HW components
component NIC_RPi_Mailbox   nwDriverMailbox;
component NIC_RPi_USB       nwDriverUsb;
// Instantiate a NetworkStack
component NetworkStack      nwStack;

// Connect interfaces USED by driver
NIC_RPi_INSTANCE_CONNECT(
    nwDriver,
    nwDriverMailbox,
    nwDriverUsb
)
// Connect interface PROVIDED by driver via the if_OS_Nic
// interface macro to the network stack
IF_OS_NIC_CONNECT(
    nwDriver,
    nic,
    nwStack,
    nic,
    event_tick_or_data)
```

#### Configuring the Component

With the code below, the driver (and the HW instances) are configured
with a DMA buffer size of 16k:

```c
// Allow for self-configuration
NIC_RPi_Mailbox_INSTANCE_CONFIGURE_SELF(
    nwDriverMailbox
)
NIC_RPi_USB_INSTANCE_CONFIGURE_SELF(
    nwDriverUsb
)
// Assign DMA buffer size
NIC_RPi_INSTANCE_CONFIGURE(
    nwDriver,
    4*4096
)
```

### Using the Component's Interfaces in C

Below are the parts of an example network stack component that uses the
NIC_RPi component:

```c
...

// For CAmkES generated interface
#include <camkes.h>

static NetworkStack_CamkesConfig_t camkes_cfg =
{
    .wait_loop_event = event_tick_or_data_wait,

    ...

    .drv_nic =
    {
        // NIC -> Stack
        .from = OS_DATAPORT_ASSIGN_SIZE(nic_from_port, NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS),
        // Stack -> NIC
        .to = OS_DATAPORT_ASSIGN(nic_to_port),

        .rpc =
        {
            .dev_read = nic_rpc_rx_data,
            .dev_write = nic_rpc_tx_data,
            .get_mac = nic_rpc_get_mac_address,
        }
    },

    ....
};

...

int run()
{
    ...
    NetworkStack_run(&camkes_cfg, ...);
    ...
}
```
