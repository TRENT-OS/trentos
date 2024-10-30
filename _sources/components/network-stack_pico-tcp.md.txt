# NetworkStack_PicoTcp

## Overview

The NetworkStack_PicoTcp component offers TCP and UDP network
connectivity to any user application connected to it. It is a component that
provides the **`if_OS_Socket`** interface and as its name already indicates,
implements the [Socket API](../api/socket_api.md) by wrapping the included picoTCP
stack (see also [http://picotcp.altran.be](http://picotcp.altran.be/)).

While the implementation is largely event-driven, the TCP timeouts require a
periodic tick from a timer component to update the internal state machine.

One network stack instance can be used by multiple applications or each
application can use a dedicated NetworkStack_PicoTcp instance. Some restrictions
apply, e.g.:

- Each network stack instance needs a dedicated NIC driver.

### Concepts

Network access is realized via sockets, which are inspired by BSD sockets but
are adapted to the CAmkES RPC interface. All socket functions are implemented
non-blocking, to reduce negative blocking between several client components all
using one NetworkStack_PicoTcp instance.

To allow operations without polling, the component offers notifications to each
client upon network activity of sockets associated with the individual client.

### Architecture

```{mermaid}
flowchart TD
    %% TRENTOS NetworkStack_PicoTcp Architecture
    subgraph TRENTOS_NetworkStack_PicoTcp_Architecture["TRENTOS NetworkStack_PicoTcp Architecture"]
        subgraph NIC_Driver["NIC Driver"]
            nic_driver["Ethernet Driver"]
        end

        TimeServer("Time Server")

        subgraph Network_Stack["Network Stack"]
            NwStack["NetworkStack_PicoTcp"]
        end

        subgraph frame_userApplication["User Application"]
            userApplication["User Application"]
        end

        NwStack -- "if_OS_Nic" --> nic_driver
        NwStack -- "if_OS_Socket" --> userApplication
        NwStack --> TimeServer
    end
```

The Networkstack_PicoTcp has one **`if_OS_Nic`** interface towards one NIC
driver.

It can offer the **`if_OS_Socket`** interface for up to 8 client components.

Internally it contains an additional CAmkES component called "Ticker". This
component provides a cyclical event tick to the NetworkStack_PicoTcp component.

For a basic configuration, the component offers one
**`if_NetworkStack_PicoTcp_Config`** interface. This can be used by a client or a
separate component to send IP address and network configuration data to the
NetworkStack_PicoTcp component.

### Implementation

The component wraps the picoTCP library internally to provide the
networking functionalities through the [Socket API](../api/socket_api.md).

It works cyclically on events - it is driven by an internal 1-second
periodic ticker, the **`if_OS_Nic`** notification received from the
connected NIC Driver component, or the RPC function calls received from
client components. The main purpose of the periodic ticker is to
guarantee socket and connection management, like timeouts, also in case
of no other external activity.

The NetworkStack_PicoTcp component implements a simple state machine.
The states are:

| State         | Description                                                                                                                                                                                                           | Specifics for API / RPC Calls                                                                         |
|---------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------|
| UNINITIALIZED | The initial state after startup.<br>The network stack will remain in this state until it has performed its basic configuration and it has received valid IP configuration data. Will switch to INITIALIZED afterward. | OS_Socket_getStatus() works<br>All other OS_Socket_xxx will be answered with OS_ERROR_NOT_INITIALIZED |
| INITIALIZED   | Network stack is fully configured, will automatically switch to RUNNING                                                                                                                                               | OS_Socket_getStatus() works<br>All other OS_Socket_xxx will be answered with OS_ERROR_NOT_INITIALIZED |
| RUNNING       | The Network stack is operational.                                                                                                                                                                                     | All OS_Socket_xxx work                                                                                |
| FATAL_ERROR   | Some unrecoverable error happened. All functions seized.                                                                                                                                                              | OS_Socket_getStatus() works<br>All other OS_Socket_xxx will be answered with OS_ERROR_ABORTED         |

Before using the [Socket API](../api/socket_api.md) a client should check for the
network stack instance being in **`RUNNING`** state.

To ease the use of the NetworkStack_PicoTcp component, several macros are
available that are shown in the following chapters.

## Usage

This is how the component can be instantiated in the system.

### Declaration of Component in CMake

The NetworkStack_PicoTcp can be declared via a simple macro in the
**`CMakeLists.txt`** file. The name you give in brackets is the name of the
component you can use later to define the instances in the CAmkES
configuration.

```CMake
NetworkStack_PicoTcp_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

If you want to set the IP configuration of the component hardcoded, you
can do so by adding compile flags as follows:

```CMake
NetworkStack_PicoTcp_DeclareCAmkESComponent(
    <NameOfComponent>
    C_FLAGS
        -DNetworkStack_PicoTcp_USE_HARDCODED_IPADDR
        -DDEV_ADDR=<ip_address>
        -DGATEWAY_ADDR=<gateway_address>
        -DSUBNET_MASK=<subnet_mask>
)
```

Additionally, the NetworkStack_PicoTcp component can also be connected
to a component that acts as a log server providing the
[Logger API](../api/logger_api.md) to the system. To connect the NetworkStack_PicoTcp
component to such a log server and set the desired log level, the component will
need to be declared as seen below:

```CMake
NetworkStack_PicoTcp_DeclareCAmkESComponent(
    <NameOfComponent>
    C_FLAGS
        -DUSE_LOGSERVER
        -DLOGSERVER_CLIENT_LEVEL_FILTER=Debug_LOG_LEVEL_INFO
        -DLOGSERVER_DATAPORT=logServer_port
    LIBS
        os_logger
)
```

### Configuration of Component in system_config.h

The define **`OS_NETWORK_MAXIMUM_SOCKET_NO`** needs to be specified for
the NetworkStack_PicoTcp. This parameter defines the maximum number of
sockets the component will be capable to manage at a time. If you
develop e.g. a TCP client application you can select this number quite
low, even just one socket would be sufficient. But if you are developing
a TCP server application or an application in which several components
require network connections in parallel, you should select a higher
number, according to your needs. If the number of open sockets reaches
this limit, calls to **`OS_Socket_create()`** or
**`OS_Socket_accept()`** will fail.

For the communication with the NIC driver, the parameter
**`NIC_DRIVER_RINGBUFFER_SIZE`** needs to be specified. It is used by
the macro **`NetworkStack_PicoTcp_COMPONENT_DEFINE`** (see below) and by
the NIC Driver Component to align the size of the dataport between them.

```c
// Required for NetworkStack_PicoTcp
#define OS_NETWORK_MAXIMUM_SOCKET_NO 32

// Required for NIC Driver and NetworkStack_PicoTcp
#define NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS 16
#define NIC_DRIVER_RINGBUFFER_SIZE \
    (NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS * 4096)
```

### Instantiation and Configuration in CAmkES

In order to use the NetworkStack_PicoTcp in your application, the
component has to be declared, instantiated, connected to a NIC driver
and the clients, and the instance itself needs to be configured in the
main CAmkES composition of the system.

#### Declaring the Component

Before the assembly block in the CAmkES file, the following needs to be
included. This will create the component definition with the desired
parameters.

```c
#include "NetworkStack_PicoTcp/camkes/NetworkStack_PicoTcp.camkes"
NetworkStack_PicoTcp_COMPONENT_DEFINE(
    <NameOfComponent>,
    NIC_DRIVER_RINGBUFFER_SIZE,
    NetworkStack_PicoTcp_NO_ADDITIONAL_INTERFACES)
```

**`NetworkStack_PicoTcp_NO_ADDITIONAL_INTERFACES`** may be replaced with any
string. The string will be placed into the main body of the component
definition. Additional interfaces may be included in the component in this way.

#### Instantiating and Connecting the Component

The NetworkStack_PicoTcp component can be instantiated and connected as follows:

- The **`NetworkStack_PicoTcp_INSTANCE_CONNECT`** macro connects the network
    stack instance to a NIC driver instance.
- The **`NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS`** macro connects
    the instance to up to 8 clients. The clients need to use the macro
    **`IF_OS_SOCKET_USE(<prefix>)`** in their component definitions. The prefix
    used needs to be repeated here (in this case "networkStack"). The prefix is
    used on the client-side to distinguish several connected network stack
    instances.
- Lastly, the NetworkStack_PicoTcp component requires a connection to
    a timeserver. This is done by using the endpoints
    **`<instance>.timeserver_rpc`** and **`<instance>.timeserver_notify`**
    with the macro **`TimeServer_INSTANCE_CONNECT_CLIENTS`**.

```c
component   <NameOfComponent> <nameOfInstance>;

NetworkStack_PicoTcp_INSTANCE_CONNECT(
    <nameOfInstance>,
    <nic_driver_instance>
)

NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>,
    <client_1_instance>, <client_1_if_OS_Socket_prefix>,
    <client_2_instance>, <client_2_if_OS_Socket_prefix>
)

// The network stack requires a timeserver connection
TimeServer_INSTANCE_CONNECT_CLIENTS(
    <timeServer_instance>,
    <nameOfInstance>.timeServer_rpc, <nameOfInstance>.timeServer_notify
)
```

#### Configuring the Instance9

Within the configuration block of the CAmkES file, the following settings are
required.

**`NetworkStack_PicoTcp_INSTANCE_CONFIGURE_CLIENTS`** will define for each
client a maximum socket number. So a client may not hold more than this number
of parallel open sockets at a time. If the number of open sockets reaches this
limit, calls to **`OS_Socket_create()`** or **`OS_Socket_accept()`** will fail
for that client. Please note, that **`OS_NETWORK_MAXIMUM_SOCKET_NO`** will be
checked independently, too. So if the network stack instance reaches this limit,
a client will not be able to open additional sockets, even if his individual
limit is not reached.

**`NetworkStack_PicoTcp_CLIENT_ASSIGN_BADGES`** will assign custom badges to
the client applications RPC endpoints. This can be used for easier debugging and
is optional.

As the NetworkStack_PicoTcp requires a TimeServer connection, the endpoint
**`<instance>.timeserver_rpc`**  needs to be included in the
**`TimeServer_CLIENT_ASSIGN_BADGES`** macro.

```c
NetworkStack_PicoTcp_INSTANCE_CONFIGURE_CLIENTS(
    <nameOfInstance>,
    <client_1_socket_limit>,
    <client_2_socket_limit>
)

// optional
NetworkStack_PicoTcp_CLIENT_ASSIGN_BADGES(
    <client_1_instance>, <client_1_if_OS_Socket_prefix>,
    <client_2_instance>, <client_2_if_OS_Socket_prefix>
)

TimeServer_CLIENT_ASSIGN_BADGES(
    <nameOfInstance>.timeServer_rpc
)
```

#### Client side setup

The client component needs to include the macro **`IF_OS_SOCKET_USE`** in its
component definition together with a prefix. The prefix is a unique identifier
for that client connection to a specific network stack instance. It is being
used also in other macros like
**`NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS`**,
**`NetworkStack_PicoTcp_CLIENT_ASSIGN_BADGES`**, **`IF_OS_SOCKET_ASSIGN`**.

```c
#include <if_OS_Socket.camkes>

component Client1 {
    IF_OS_SOCKET_USE(<if_OS_Socket_prefix>)
}
```

In the implementation, the client needs to set up the RPC lookup table with the
macro **`IF_OS_SOCKET_ASSIGN`** and the \<prefix\> the client used also for
**`IF_OS_SOCKET_USE`**.

The created variable (in the example below named "**`networkStackCtx`**") can
then be used in the **`OS_Socket_xxx()`** functions provided by the
[Socket API](../api/socket_api.md) as the ctx parameter.

```c
#include "OS_Error.h"

#include "OS_Socket.h"
#include "interfaces/if_OS_Socket.h"
#include <camkes.h>

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(<if_OS_Socket_prefix>);

...
```

## Example - Basic Setup

In this example, we connect two client components to the network stack. In this
chapter, we are doing the basic setup. The following chapters will show, how a
client application can communicate with the network stack instance.

### Instantiation of Component in CMake

```CMake
NetworkStack_PicoTcp_DeclareCAmkESComponent(
    NetworkStack_PicoTcp
    C_FLAGS
        -DNetworkStack_PicoTcp_USE_HARDCODED_IPADDR
        -DDEV_ADDR="10.0.0.11"
        -DGATEWAY_ADDR="10.0.0.1"
        -DSUBNET_MASK="255.255.255.0"
)
```

### Configuration of Component in system_config.h

```c
#define OS_NETWORK_MAXIMUM_SOCKET_NO 32

#define NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS 16
#define NIC_DRIVER_RINGBUFFER_SIZE \
    (NIC_DRIVER_RINGBUFFER_NUMBER_ELEMENTS * 4096)
```

### Instantiation and Configuration in CAmkES

In this main application CAmkES file example, the NetworkStack_PicoTcp instance
is connected to a [NIC_iMX6](../components/nic_imx6.md) component and two client
applications.

```c
#include "system_config.h"

#include "TimeServer/camkes/TimeServer.camkes"
TimeServer_COMPONENT_DEFINE(TimeServer)

#include "NetworkStack_PicoTcp/camkes/NetworkStack_PicoTcp.camkes"
NetworkStack_PicoTcp_COMPONENT_DEFINE(
    NetworkStack_PicoTcp,
    NIC_DRIVER_RINGBUFFER_SIZE,
    NetworkStack_PicoTcp_NO_ADDITIONAL_INTERFACES)

#include "NIC_iMX6/NIC_iMX6.camkes"

#include "Client1.camkes"
#include "Client2.camkes"

assembly {
    composition {
        component TimeServer timeServer;
        NIC_IMX6_INSTANCE(nwDriver)
        component NetworkStack_PicoTcp nwStack;
        component Client1 client1;
        component Client2 client2;

        TimeServer_INSTANCE_CONNECT_CLIENTS(
            timeServer,
            nwStack.timeServer_rpc, nwStack.timeServer_notify
        )

        NetworkStack_PicoTcp_INSTANCE_CONNECT(
            nwStack,
            nwDriver
        )

        NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS(
            nwStack,
            client1, networkStack,
            client2, networkStack
        )
     }

    configuration {
        TimeServer_CLIENT_ASSIGN_BADGES(
            nwStack.timeServer_rpc
        )

        NetworkStack_PicoTcp_CLIENT_ASSIGN_BADGES(
            client1, networkStack,
            client2, networkStack
        )

        NetworkStack_PicoTcp_INSTANCE_CONFIGURE_CLIENTS(
            nwStack,
            24,
            16
        )

        NIC_IMX6_MEMORY_CONFIG(nwDriver)
    }
}
```

### Client Component Definitions

The clients component definition needs to include **`IF_OS_SOCKET_USE()`** as
shown below.

```c
#include <if_OS_Socket.camkes>

component Client1 {
    control;
    IF_OS_SOCKET_USE(networkStack)
}
```

### Using the Component's Interfaces in C

For a client to talk to the network stack, it needs to set up the RPC
lookup table with the macro **`IF_OS_SOCKET_ASSIGN`** and the \<prefix\>
the client used also for **`IF_OS_SOCKET_USE`**.

```c
#include "OS_Error.h"

#include "OS_Socket.h"
#include "interfaces/if_OS_Socket.h"
#include <camkes.h>

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(networkStack);

...
...
...
OS_Socket_Handle_t socket;

err = OS_Socket_create(
    &networkStackCtx,
    &socket,
    OS_AF_INET,
    OS_SOCK_STREAM);

err = OS_Socket_connect(socket, &dstAddr);
...
...
...
```

## Example - Use of the Configuration Interface

Instead of defining the IP configuration at compile time by the use of
**`-DNetworkStack_PicoTcp_USE_HARDCODED_IPADDR`**, one can also use the
configuration interface.

The following example will introduce a new component called
NwStackConfigurator, which will make use of the
**`if_NetworkStack_PicoTcp_Config`** interface of the network stack
instance. Through this connection, the NwStackConfigurator component
will send the IP configuration data to the NetworkStack_PicoTcp
instance at runtime. The component itself receives this data at compile
time, but could also read that data from a configuration file or get it
from a configuration service.

In CMake we define the following component declaration:

```c
DeclareCAmkESComponent(
    NwStackConfigurator
    SOURCES
        components/NwStackConfigurator/NwStackConfigurator.c
    C_FLAGS
        -Wall
        -Werror
        -DNWSTACK_DEV_ADDR="10.0.0.11"
        -DNWSTACK_GATEWAY_ADDR="10.0.0.1"
        -DNWSTACK_SUBNET_MASK="255.255.255.0"
    LIBS
        system_config
        os_core_api
        lib_debug
        NetworkStack_PicoTcp_api
)
```

The component definition is included in **`NwStackConfigurator.camkes`**. It
defines a component with one interface only:

```c
#include "NetworkStack_PicoTcp/camkes/if_NetworkStack_PicoTcp_Config.camkes"

component NwStackConfigurator {
    if_NetworkStack_PicoTcp_Config_USE(networkStack_config)
}
```

Within the main application CAmkES file, we need to instantiate the component.
The connection to the network stack instance is done via the macro
**`NetworkStack_PicoTcp_INSTANCE_CONNECT_CONFIG_CLIENT`**. The prefix
**`networkStack_config`** needs to be the same as in the component definition.

```c
#include "components/NwStackConfigurator/NwStackConfigurator.camkes"

assembly {
    composition {
        ....
        component NwStackConfigurator nwStackConfigurator;

        NetworkStack_PicoTcp_INSTANCE_CONNECT_CONFIG_CLIENT(
            nwStack,
            nwStackConfigurator, networkStack_config
        )
        ....
```

The implementation of **`NwStackConfigurator.c`** first creates the RPC lookup
table **`networkStackConfigCtx`** by using the macro
**`if_NetworkStack_PicoTcp_Config_ASSIGN`** together with the prefix used
before.

To send the configuration data to the network stack the RPC call
referred to by **`networkStackConfigCtx.configIpAddr()`** is used.

```c
#include "OS_Error.h"
#include "OS_Socket.h"
#include "if_NetworkStack_PicoTcp_Config.h"

#include "lib_debug/Debug.h"
#include <camkes.h>

//------------------------------------------------------------------------------
static const if_NetworkStack_PicoTcp_Config_t networkStackConfigCtx =
    if_NetworkStack_PicoTcp_Config_ASSIGN(networkStack_config);


//------------------------------------------------------------------------------
void
post_init(void)
{
    static const OS_NetworkStack_AddressConfig_t ipAddrConfig =
    {
        .dev_addr       = NWSTACK_DEV_ADDR,
        .gateway_addr   = NWSTACK_GATEWAY_ADDR,
        .subnet_mask    = NWSTACK_SUBNET_MASK
    };

    OS_Error_t err = networkStackConfigCtx.configIpAddr(&ipAddrConfig);
    Debug_ASSERT(err == OS_SUCCESS);
}
```
