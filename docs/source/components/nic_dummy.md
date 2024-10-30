# NIC_Dummy

## Overview

The NIC_Dummy implements a dummy driver that exposes the **`if_OS_Nic`**
interface.

The driver does not implement any functionality, its purpose is just to
act as a dummy network interface for development purposes. It can be
used for prototyping software for platforms where an actual NIC driver
is not yet available.

### Implementation

The implementation of the **`if_OS_Nic`** interface is complete but effectively the
component only "pretends" to work, in other words it never returns an error but:

- writes to NIC_Dummy effectively do nothing,
- NIC_Dummy never signals any incoming data,
- NIC_Dummy always returns **`{0x00, 0x11, 0x22, 0x33, 0x44, 0x55}`**
    as MAC address.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

A NIC_Dummy driver is defined as:

```CMake
NIC_Dummy_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

### Instantiation and Configuration in CAmkES

Here we show how to instantiate and connect the component.

#### Declaring the Component

The corresponding NIC_Dummy component is defined in CAmkES as follows:

```c
#include "NIC_Dummy/NIC_Dummy.camkes"
NIC_Dummy_COMPONENT_DEFINE(<NameOfComponent>)
```

### Instantiating and Connecting the Component

To instantiate the NIC_Dummy (and optionally connect it to the logger)
the following can be used:

```c
component <NameOfComponent>   <nameOfInstance>;

NIC_Dummy_INSTANCE_CONNECT_OPTIONAL_LOGGER(
    <nameOfInstance>,
    <logServer>.<log_rpc>, <logServer>.<log_port>
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

## Example

Here we see how to use the NIC_Dummy. Usually, it would be connected to
the [Socket API](../api/socket_api.md) through a network stack component.

### Instantiation of the Component in CMake

The NIC_Dummy can simply be added to the build like this:

```CMake
NIC_Dummy_DeclareCAmkESComponent(
    MyNIC_Dummy
)
```

### Instantiation and Configuration in CAmkES

In this example, the component is connected to a second component which
acts as network stack.

#### Declaring the Component

The NIC_Dummy can be declared as follows:

```c
#include "NIC_Dummy/NIC_Dummy.camkes"
NIC_Dummy_INSTANCE_CONNECT(MyNIC_Dummy)
```

#### Instantiating and Connecting the Component

To connect the NIC_Dummy with a network stack component, the following
code can be used:

```c
// Instantiate the NIC_Dummy driver
component  MyNIC_Dummy      nwDriver;
// Instantiate NetworkStack component
component  NwStack          nwStack;
// Component LogServer component
component  LogServer        logServer;

// Connect OPTIONAL interface used by driver
NIC_Dummy_INSTANCE_CONNECT_OPTIONAL_LOGGER(
    nwDriver,
    logServer.log_server_interface, logServer.dataport_buf_nwDriver
)
// Connect the driver to the network stack instance using the macro
// provided by the if_OS_Nic interface
IF_OS_NIC_CONNECT(
    nwDriver,
    nic,
    nwStack,
    nic,
    event_tick_or_data)
```

#### Using the Component's Interfaces in C

Below are the parts of an example network stack component that uses the
NIC_Dummy component:

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
