# NIC_iMX6

## Overview

The NIC_iMX6 driver provides ethernet functionality in NXP's i.MX6
family of SOCs. Currently, two members are explicitly supported

- the i.MX6Quad on the BD-SL-i.MX6 (formerly called Sabre Lite) board
    from Boundary Devices, which has one Ethernet port
- the i.MX6SoloX on the Nitrogen6_SoloX from Boundary Devices, which
    has two Ethernet ports.

The CAmkES NIC_iMX6 component contains the ethernet driver for these
boards. The drivers themselves provide the **`if_OS_Nic`** interface.
For each ethernet port, one dedicated instance of the driver is
required.

## Hardware

The CAmkES driver NIC_iMX6 is specifically built to support the
platforms mentioned above, as some hardware dependencies are hardcoded:

- BD-SL-i.MX6
  - it uses a Micrel KSZ9021 PHY chip, where the required commands
        are hard-coded in the driver.
- Nitrogen6_SoloX
  - each of the two ports uses its own Atheros AR8035 PHY chips,
        where the commands are hard-coded in the driver.
  - the SmartEEE feature of the Atheros AR8035 PHY is disabled
        because it causes glitches in the ethernet link.
  - both PHYs are connected to the management interface of the first
        ethernet port, thus the driver for the second port needs to
        access the driver for the first port for PHY communication.

While the NIC_iMX6 driver could also be used on other i.MX6 platforms,
this has not been tested and might require further adaptation.

### Implementation

The NIC_iMX6 driver consists of two parts:

- The seL4 platform support library that offers functionality to deal
    with the low-level access to the ethernet peripherals and PHY chip
- The TRENTOS specific wrapper driver that uses the library
    internally. It implements:
  - TRENTOS specific features (e.g. interfaces, memory allocation,
        error handling, logging)
  - platform specific configuration for the ethernet ports
  - synchronization between the network ports in case of the
        Nitrogen6_SoloX
  - network capabilities to a network stack running on top through
        the **`if_OS_Nic`** interface

When building an application that uses both ethernet ports on the
Nitrogen6_SoloX, the first port takes the role of a master port
concerning the MDIO bus for communication with the PHY. This is due to
the hardware implementation of the Nitrogen6_SoloX boards, both PHYs
are to the same management interface. The NIC_iMX6 implements an
interface between the two driver instances, where the second ethernet
port's driver can sends commands to the first driver to communicate
with it's PHY. Furthermore, this interface is used for internal driver
synchronization during the initialization sequence, as the second driver
can only start once the first driver has finished starting.

### Limitations

- Each NIC driver must be connected to exactly one network stack, or
    any component using the provided **`if_OS_Nic`** interface.
- On the Nitrogen6_SoloX only the following setups are supported:
  - Ethernet port 1 is used alone (only one instance of NIC_iMX6
        driver is created).
  - Ethernet port 1 and 2 are used (two instances of NIC_iMX6
        driver are created).
  - The second ethernet port cannot be used alone.
- Auto-negotiation of the link characteristics (speed and duplex mode)
    happens at system startup. If during operations the state of the
    link changes (e.g. the cable is unplugged), the network stack will
    experience packet loss and receive/transmit errors. Neither the NIC
    driver nor the network implements the concept of link state changes
    during runtime. If the cable is plugged in again, there is no
    re-negotiation about the link, so there is no guarantee the
    connection can be re-established.

### Specifics

#### User Defined MAC Addresses

The NIC_iMX6 can either use the MAC address from efuses/OCOTP of the
i.MX6 chip or it can be set to user defined MAC address via the CAmkES
configuration. For the CAmkES configuration option, this is a compile
time setting. If several boards are to be used with different specific
MAC addresses, an individual TRENTOS system image must be created for
each board.

#### MAC Address for Port 2 on Nitrogen6_SoloX

On the Nitrogen6_SoloX boards that were available for testing, only the
MAC address of ethernet port 1 is set in eFuses/OCOTP by the
manufacturer. The MAC address for the second ethernet port is all set to
zeros. The NIC_iMX6 driver handles this by using the MAC address of
port 1 and increments it by 1 for the second ethernet port. While this
approach can also be found elsewhere, one must keep in mind that the
manufacturer may use consecutive MAC addresses on these boards. Such
cases could also be observed. This can lead to the case, where the
ethernet port 2 of one board can get assigned a MAC address also used on
another Nitrogen6_SoloX board for ethernet port 1.

## Usage

### Declaration of the Component in CMake

For simplicity and portability it is recommended to declare component
types for all available NICs with the following function:

```CMake
DeclareCAmkESComponents_for_NICs()
```

### Instantiation and Configuration in CAmkES

In order to use the NIC_iMX6 network driver in a CAmkES system, it has
to be declared, instantiated, and connected to a number of components
(network stack, logger).

#### Declaring the CAmkES Component

The NIC_iMX6 driver is automatically declared when it is being
instantiated. Each instance is alone responsible for a specific hardware
resource, which is handled internally. This avoids the problem of
multiple instances trying to manage the same hardware peripheral.

#### Instantiating and Connecting the CAmkES Component

In your system's CAmkES configuration file you need to do the following
steps:

- include the driver CAmkES definitions, found in
    **`<sdk>/components/NIC_iMX6/NIC_iMX6.camkes`**
- instantiate one or two driver instances via the macro
    **`NIC_IMX6_INSTANCE()`**
  - instantiating the driver for two ports on the BD-SL-i.MX6
        platform will lead to an error at compile time
  - on the Nitrogen6_SoloX platform, you can instantiate the driver for either
        one port or for both. If the driver is instantiated for a single port,
        the primary one will be used
- connect the driver instance to a network stack instance
  - each driver instance needs to be connected to a networks stack
        instance. Only a 1-to-1 mapping is supported
- connect the driver instance to the LogServer (optional interface)
  - to write its information to the log, each driver instance should
        be connected to the LogServer instance

```c
// to include CAmkES driver definitions
#include "NIC_iMX6/NIC_iMX6.camkes"

// declaring and instantiating a single ethernet driver instance on the primary port
NIC_IMX6_INSTANCE(<NameOfNICComponent>)

// declaring and instantiating two ethernet drivers, one for each port
NIC_IMX6_INSTANCE(<NameOfNICComponent_Port1>, <NameOfNICComponent_Port2>)

// connecting one network driver instance to one logserver instance
NIC_IMX6_INSTANCE_CONNECT_OPTIONAL_LOGGER(<NameOfNICComponent>, <NameOfLogserverComponent>)
```

#### Configuring the Instance

The default configuration for each network driver instance creates the
following configuration:

- sets up the memory needed by the network driver
- configures the driver to use the MAC address stored in the hardware
- connects the driver to its corresponding PHY chip
- in case of the Nitrogen6_SoloX dual port configuration, it sets up
    the synchronization routine between the two ports

```c
# configuring a system using a single ethernet driver instance, on the primary port
NIC_IMX6_MEMORY_CONFIG(<NameOfNICComponent>)

# configuring a system using both ethernet ports
NIC_IMX6_MEMORY_CONFIG(<NameOfNICComponent_Port1>)
NIC_IMX6_MEMORY_CONFIG(<NameOfNICComponent_Port2>)
```

For each network driver instance the following CAmkES attributes are
available:

|                                                                                                Field                                                                                               |                                                                                                                                                                                                                                                Value                                                                                                                                                                                                                                               |
|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|
|  <NameOfNICComponent>.MAC_address                                                                                                                                                                  | Sets the MAC address of the current network driver instance. The default value of all 0 instructs the driver to read the MAC address from the hardware efuses/OCOTP.<br>Values:<br>" \x00\x00\x00\x00\x00\x00 ": read MAC from efuses (default value)<br>other: use specified MAC Address                                                                                                                                                                                                          |
|  <NameOfNICComponent>.nic_phy_address                                                                                                                                                              | Address of the PHY chip in the MDIO bus.<br>Warning: These are platform specific configuration parameters and shouldn't be changed by the user.<br>BD-SL-i.MX6: use 0 to auto-detect the PHY or specify an address.<br>Nitrogen6_SoloX: the PHY address for ethernet port 1 is 4, the address for ethernet port 2 is 5. Note that any setting for the second port is ignored, as the phy address 5 is hard-coded in the driver.                                                                    |
|  <NameOfNICComponent>.nic_id                                                                                                                                                                       | ID of the Ethernet peripheral.<br>Warning: These are platform specific configurations and shouldn't be changed by the user.<br>BD-SL-i.MX6: value is 0<br>Nitrogen6_SoloX: value is 0 for ethernet port 1 and 1 for ethernet port 2.                                                                                                                                                                                                                                                               |
|  <NameOfNICComponent>.nic_flags                                                                                                                                                                    | The following NIC configuration flags are available:<br>NIC_CONFIG_PROMISCUOUS_MODE: Don't pass the Ethernet CRC fields to the network stack.<br>NIC_CONFIG_DROP_FRAME_CRC: Pass all frames received to the network stack, regardless of the destination MAC address. Warning: The network stack might not be able to cope with a high amount of traffic.<br>The current driver implementation is tested without the promiscuous mode set.                                                         |
|  <NameOfNICComponent>.simple  <br> <NameOfNICComponent>.cnode_size_bits  <br> <NameOfNICComponent>.simple_untyped20_pool  <br> <NameOfNICComponent>.heap_size  <br> <NameOfNICComponent>.dma_pool  | These parameters set up the memory needed by the driver instance.<br>Warning: These values are tailored to the current driver implementation and shouldn't be changed by the user.<br>Default Values:<br>                  simple = true                <br>                  cnode_size_bits = 12                <br>                  simple_untyped20_pool = 2                <br>                  heap_size = 0x10000                <br>                  dma_pool = 0xC2000                 |

## Example

The normal usage pattern of the NIC_iMX6 network driver is connecting
it to a network stack instance, which then provides the
[Socket API](../api/socket_api.md) to an application.

### Instantiation and Configuration in CAmkES

In this example, the network driver component instance is connected to a
network stack component instance and an optional LogServer instance.

#### Instantiating and Connecting the Component

To connect the NIC_iMX6 driver with a network stack component the
following patterns can be used.

The first example is for using a single ethernet port on the
BD-SL-i.MX6 or on the Nitrogen6_SoloX:

```c
composition {

...

 // Instantiate network stack component
 component NwStack nwStack;

 // Instantiate logserver component
 component LogServer logServer;

 // Instantiate the iMX6 driver on the primary ethernet port
 NIC_IMX6_INSTANCE(nwDriver)

 // Connect the driver to the network stack instance using the macro
    // provided by the if_OS_Nic interface
 IF_OS_NIC_CONNECT(nwDriver, nic, nwStack, nic, event_tick_or_data)

 // Connect the drivers to the logserver instance
 NIC_IMX6_INSTANCE_CONNECT_OPTIONAL_LOGGER(nwDriver, logServer)

...

}
configuration
{

...
 // set the memory configuration of the driver instance
 NIC_IMX6_MEMORY_CONFIG(nwDriver)
 // set a custom mac address
 nwDriver.MAC_address = "\xDE\xAD\xDE\xAD\xDE\AD"

...
}
```

The second example is for using both ethernet ports on the Nitrogen6_SoloX.
In this configuration, the nwDriver1 corresponds to the primary port.

```c
composition {

...

 // Instantiate network stack components
 component  NwStack    nwStack1;
 component  NwStack    nwStack2;

 // Instantiate logserver component
 component  LogServer  logServer;

 // Instantiate the iMX6 driver on the primary ethernet port
 NIC_IMX6_INSTANCE(nwDriver1, nwDriver2)

    // Connect the drivers to the network stack instances
    IF_OS_NIC_CONNECT(nwDriver1, nic, nwStack1, nic, event_tick_or_data)
    IF_OS_NIC_CONNECT(nwDriver2, nic, nwStack2, nic, event_tick_or_data)

 // Connect the drivers to the logserver instance
 NIC_IMX6_INSTANCE_CONNECT_OPTIONAL_LOGGER(nwDriver1, logServer)
 NIC_IMX6_INSTANCE_CONNECT_OPTIONAL_LOGGER(nwDriver2, logServer)

...

}
configuration
{

...
 // set the memory configuration of the driver instance
 NIC_IMX6_MEMORY_CONFIG(nwDriver1)
 // set the memory configuration of the driver instance
 NIC_IMX6_MEMORY_CONFIG(nwDriver2)

 // set a custom mac address
 nwDriver1.MAC_address = "\xDE\xAD\xDE\xAD\x00\00"

 // set a custom mac address
 nwDriver2.MAC_address = "\xAD\xAD\xAD\xDE\x00\01"

...
}
```

### Using the Component's Interfaces in C

Below are the parts of an example network stack component that uses the
NIC_iMX6 driver component:

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
