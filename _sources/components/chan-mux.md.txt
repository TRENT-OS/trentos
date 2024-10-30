# ChanMux

## Overview

The ChanMux component multiplexes an arbitrary number of full-duplex
communication channels from clients over a single channel. ChanMux is agnostic
of the upper and lower channel details. In terms of software interfaces, ChanMux
abstracts its lower layer by exposing a stream interface. A lower physical
channel could be, for example, an UART. The lower layer protocol currently
implements a packet-oriented data link protocol (HDLC) with a maximum packet
size (MTU).

One major use case of ChanMux is development support when working with TRENTOS.
The TRENTOS SDK includes **trentos_build** and **trentos_test** docker
containers, in which the TRENTOS application runs in QEMU emulating the target
system. This has many benefits, especially being hardware agnostic. One drawback
of this is limited driver support. ChanMux can help with that, as it can
forward all data communication to the host platform, where access to
hardware resources is already given.

The TRENTOS SDK docker containers are set up so that ChanMux within QEMU
can communicate over UART with the host machine. The QEMU service offers
a virtualized UART (**Host Bridge**) that can be connected to a
pseudo-terminal (PTY) or a TCP socket on the host. The TRENTOS SDK
includes the so-called \"Proxy\" application, which is running on the
host and connects to one of these. The proxy application provides a set
of dedicated channels, which can emulate various hardware devices like
persistent storage or network. ChanMux muxes these channels into the
UART and the proxy application demuxes the channels and implements the
device emulation.

In the current implementation of the proxy application, the assignment
of the channels is static and must be defined at compile time. In the
case of the NVM channels, this even means that the size of the NVM on a
certain channel is fixed. Channel definition must be kept in sync
between the proxy application and ChanMux of a TRENTOS system.

ChanMux implements one FIFO per channel for incoming data from the lower
interface. If the channel FIFO is not drained by the application, an
overflow condition will be reported to the application. ChanMux does not
implement any flow control, the application must handle this by defining
a proper protocol that is used in the channel. In case of an NVM
channel, a simple request-response protocol is used, where the
application is in full control. In the case of the network interface,
the Ethernet frames are simply dropped if the FIFO has not enough space.

### Architecture

```{mermaid}
flowchart TD
    %% TRENTOS on QEMU Node
    subgraph TRENTOS["TRENTOS on QEMU"]
        subgraph App1["App 1"]
            App_Chan1[App_Chan1]
            ChanMuxClient_1["ChanMuxClient"]
        end
        subgraph App2["App 2"]
            App_Chan2[App_Chan2]
            ChanMuxClient_2["ChanMuxClient"]
        end
        subgraph App3["App 3"]
            App_Chan3[App_Chan3]
            ChanMuxClient_3["ChanMuxClient"]
        end
        subgraph App4["App 4"]
            App_Chan4[App_Chan4]
            ChanMuxClient_4["ChanMuxClient"]
        end
        subgraph ChanMuxServer["ChanMux Server"]
            ChanMuxRpc[ChanMuxRpc]
            ChanMux[ChanMux]
            Uart[Uart]
        end

        App_Chan1 --> ChanMuxClient_1
        App_Chan2 --> ChanMuxClient_2
        App_Chan3 --> ChanMuxClient_3
        App_Chan4 --> ChanMuxClient_4
        ChanMuxClient_1 --> ChanMuxRpc
        ChanMuxClient_2 --> ChanMuxRpc
        ChanMuxClient_3 --> ChanMuxRpc
        ChanMuxClient_4 --> ChanMuxRpc
        ChanMuxRpc -.-> ChanMux
        ChanMux --> Uart
    end

    %% Linux Proxy App Node
    subgraph Proxy["Linux Proxy App"]
        LinuxSocket[LinuxSocket]
        ProxyApp[ProxyApp]
        EndPoint_Chan1[EndPoint_Chan1]
        EndPoint_Chan2[EndPoint_Chan2]
        EndPoint_Chan3[EndPoint_Chan3]
        EndPoint_Chan4[EndPoint_Chan4]

        LinuxSocket --> ProxyApp
        ProxyApp --> EndPoint_Chan1
        ProxyApp --> EndPoint_Chan2
        ProxyApp --> EndPoint_Chan3
        ProxyApp --> EndPoint_Chan4
    end

    %% Inter-Node Connection
    Uart <-.-> LinuxSocket
```

```{mermaid}
flowchart TD
    %% Interface
    ChanMuxRpc(ChanMuxRpc)

    %% Application Component
    subgraph Node_App["Application component"]
        Application(Application)
        ChanMuxClient(ChanMuxClient)
        Application --> ChanMuxClient
        ChanMuxClient --> ChanMuxRpc
    end

    %% ChanMux Component
    subgraph ChanMux["ChanMux component"]
        ChanMuxClass(ChanMux)
        ChanMuxStream(ChanMuxStream)
        Hdlc(Hdlc)
        ChanMuxClass --> ChanMuxStream
        ChanMuxStream --> Hdlc
    end

    %% Uart Component
    subgraph Uart["Uart component (example of underlying layer)"]
        UartClass(Uart)
    end

    %% Connections
    ChanMuxRpc -.-> ChanMuxClass
    Hdlc --> UartClass
```

### Implementation

#### ChanMuxClient

Usage concept:

- write calls are always blocking
  - it depends on the lower layer's protocol if they block until
        data has been sent (UART) or even until a receiver has confirmed
        it received the data (TCP)
  - if a caller does not like blocking, it must create a separate
        thread
  - non-blocking writes are not supported at this time
- read can be blocking
  - the MUX implementation holds the logic for this
- read can be non-blocking
  - returns immediately with the data that is available and fits
        into the provided buffer
  - the caller has to block waiting on the event for new data to
        arrive
  - the caller shall not do polling, but in the end, the MUX can\'t
        prevent it from doing so
- read and write may process less data than requested
  - the caller will be informed about this and should call the API
        again

#### Component

```c
import <if_OS_BlockingWrite.camkes>;
import <if_ChanMux.camkes>;
import <if_OS_Logger.camkes>;

// platform specific defaults
#include "ChanMux_plat_defaults.camkes"
#include "ChanMuxHelper.camkes"

#include "lib_macros/ForEach.h"

//------------------------------------------------------------------------------
// Component definitions, variable arguments are a list of pairs
// (client_instance_name, channel_name) that uniquely identify a channel
#define ChanMux_COMPONENT_DEFINE(_name_, ...) \
    \
    component _name_ { \
        \
        control; \
        /*-------------------------------------------------*/ \
        /* upper interface */ \
        provides        ChanMuxDriverInf     ChanMuxRpc; \
        FOR_EACH_0FP_2VP( \
            ChanMux_DECLARE_CHANNEL_CONNECTOR, \
            __VA_ARGS__ \
        ) \
        /*-------------------------------------------------*/ \
        /* lower interface to underlying layer */ \
        uses            if_OS_BlockingWrite  UnderlyingChan_Rpc; \
        dataport        Buf                  UnderlyingChan_inputFifoDataport; \
        dataport        Buf                  UnderlyingChan_outputDataport; \
        consumes        EventDataAvailable   UnderlyingChan_EventHasData; \
        /*-------------------------------------------------*/ \
        /* optional interface to log server */ \
        maybe dataport  Buf                  LogServer_buf; \
        maybe uses      if_OS_Logger         LogServer_rpc; \
        attribute       int                  log_lvl; \
    }


//------------------------------------------------------------------------------
// Interface definitions and connections

#define ChanMux_DECLARE_CHANNEL_CONNECTOR(_inst_client_, _chan_name_) \
    dataport  Buf                 _inst_client_ ##_## _chan_name_ ##_## portRead; \
    dataport  Buf                 _inst_client_ ##_## _chan_name_ ##_## portWrite; \
    emits     DataAvailableEvent  _inst_client_ ##_## _chan_name_ ##_## eventHasData;
```

#### Interface

```c
procedure ChanMuxDriverInf
{
    include "OS_Error.h";

    OS_Error_t
    write(
        in unsigned chan,
        in size_t len,
        out size_t written
    );

    OS_Error_t
    read(
        in unsigned chan,
        in size_t len,
        out size_t read
    );
};
```

## Usage

This is how the component can be instantiated in the system.

**Info:** We are using here the ChanMux_UART, which is a specialized
version of the ChanMux and used throughout TRENTOS to connect to various
channels to the UART.

### Declaration of the Component in CMake

The ChanMux_UART can be instantiated via the following:

```CMake
ChanMux_UART_DeclareCAmkESComponents(
    <ChanMux_UART instance>
    <ChanMux config file> // containing assigned resources to the ChanMux
    <System config file>  // containing system configuration macros
)
```

### Instantiation and Configuration in CAmkES

Here we show how to instantiate the ChanMux_UART component.

**Info:** ChanMux_UART is connected directly to the UART, so a UART
must be instantiated as well. This will be demonstrated as part of the
example. In this section, we show only the macros specific to the
ChanMux_UART component.

#### Declaring the Component

When the ChanMux_UART is declared, the channels which its clients
expect to use have to be set up as well:

```c
#include "ChanMux/ChanMux_UART.camkes"
ChanMux_UART_COMPONENT_DEFINE(
    <NameOfComponent>,
    <client>, <channel1>,
    <client>, <channel2>,
    ...
)
```

The macro takes a variable list of arguments: In this example, the first
client has two channels (e.g., a network stack). The names of the
clients and the channels will be referenced later.

#### Instantiating and Connecting the Component

The ChanMux_UART first needs to be instantiated and connected to a
UART:

```c
component <NameOfComponent> <nameOfInstance>;

ChanMux_UART_INSTANCE_CONNECT(
    <nameOfInstance>,
    <uart>
)
```

After setting up the ChanMux_UART, its respective clients can be connected.
In contrast to the usual convention, every client has to be connected
individually via **`ChanMux_UART_INSTANCE_CONNECT_CLIENT()`**, instead of
**`ChanMux_UART_INSTANCE_CONNECT_CLIENTS()`**.

```c
ChanMux_UART_INSTANCE_CONNECT_CLIENT(
    <nameOfInstance>,
    <client>, <channel1>, <channel2>,
    ...
)
```

Please note that the client\'s name and channel names need to be consistent
with the names given in the **`ChanMux_UART_COMPONENT_DEFINE()`** macro.

#### Configuring the Instance

Clients of the ChanMux_UART need to have their badges assigned, for
which the following macro can be used:

```c
ChanMux_UART_CLIENT_ASSIGN_BADGES(
    <client>.<chanmux_rpc>,
    ...
)
```

## Example

In the following example, a ChanMux component is connected to 2 clients
(tester1, tester2) and the UART as the underlying layer. The testers
will perform tests on the ChanMux abilities by streaming data to an
external application via UART.

### Instantiation of the Component in CMake

```CMake
ChanMux_UART_DeclareCAmkESComponents(
    MyChanMux_UART
    components/ChanMux/ChanMux_config.c
    system_config
)
```

### Configuration

A configuration must always be provided. This will be included and
compiled together with the component definition.

```c
#include "system_config.h"
#include "ChanMux/ChanMux.h"
#include <camkes.h>


//------------------------------------------------------------------------------
static unsigned int
resolveChannel(
    unsigned int  sender_id,
    unsigned int  chanNum_local)
{
    switch (sender_id)
    {
    //----------------------------------
    case CHANMUX_ID_TESTER_1:
        // TODO: check that chanNum_local is 0, reject anything else
        return CHANMUX_CHANNEL_TEST_1;
    //----------------------------------
    case CHANMUX_ID_TESTER_2:
        // TODO: check that chanNum_local is 0, reject anything else
        return CHANMUX_CHANNEL_TEST_2;
    //----------------------------------
    default:
        break;
    }

    return INVALID_CHANNEL;
}


//------------------------------------------------------------------------------
static uint8_t testerFifo[2][CHANMUX_TEST_FIFO_SIZE];

static ChanMux_Channel_t test_channel[2];


//------------------------------------------------------------------------------
static const ChanMux_ChannelCtx_t channelCtx[] = {

    CHANMUX_CHANNEL_CTX(
        CHANMUX_CHANNEL_TEST_1,
        &test_channel[0],
        testerFifo[0], // must be the buffer and not a pointer
        CHANMUX_DATAPORT_ASSIGN(tester1_chan_portRead, tester1_chan_portWrite),
        tester1_chan_DataAvailable_emit),

    CHANMUX_CHANNEL_CTX(
        CHANMUX_CHANNEL_TEST_2,
        &test_channel[1],
        testerFifo[1], // must be the buffer and not a pointer
        CHANMUX_DATAPORT_ASSIGN(tester2_chan_portRead, tester2_chan_portWrite),
        tester2_chan_DataAvailable_emit),

};


//------------------------------------------------------------------------------
// this is used by the ChanMux component
const ChanMux_Config_t cfgChanMux =
{
    .resolveChannel = &resolveChannel,
    .numChannels    = ARRAY_SIZE(channelCtx),
    .channelCtx     = channelCtx,
};
```

### Configuration of the Component in system_config.h

The following definitions are not directly required by ChanMux but they
are used in the **`ChanMux_config.c`** file in order to identify channel IDs
and client badges.

```c
//-----------------------------------------------------------------------------
// ChanMux channels
//-----------------------------------------------------------------------------
#define CHANMUX_CHANNEL_TEST_1          10
#define CHANMUX_CHANNEL_TEST_2          11

//-----------------------------------------------------------------------------
// ChanMux clients
//-----------------------------------------------------------------------------
#define CHANMUX_ID_TESTER_1        101
#define CHANMUX_ID_TESTER_2        102
```

**Note:** At the moment the user defines these **`CHANMUX_ID_XX`** macros
and must do it in a way that is consistent to the assignment made
by **`ChanMux_UART_CLIENT_ASSIGN_BADGES()`**. This two macros are needed to
retrieve some client context in the server space. In the future it will
be implemented with a mechanism that will map a sender ID to such a
context. In this way there will be no need to manually take care of the
consistency of the definitions.

### Instantiation and Configuration in CAmkES

Note that the component must be defined first. ChanMux cannot come with
a predefined component as the user components cannot be determined
beforehand.

```c
#include "ChanMux/ChanMux_UART.camkes"
ChanMux_UART_COMPONENT_DEFINE(
    MyChanMux_UART,
    tester1, chan,
    tester2, chan
)
#include "UART/UART.camkes"
UART_COMPONENT_DEFINE(
    MyUART,
    {"aliases":"serial0"}
)

assembly {
    composition {
        // Instantiate ChanMux_UART and UART
        component MyChanMux_UART    chanMux_UART;
        component MyUART            uart;
        // Instantiate testers
        component SingleTester      tester1;
        component SingleTester      tester2;

        // Connect ChanMux to UART
        ChanMux_UART_INSTANCE_CONNECT(
            chanMux_UART,
            uart
        )
        // Connect the single channel testers
        ChanMux_UART_INSTANCE_CONNECT_CLIENT(
            chanMux_UART,
            tester1, chan
        )
        ChanMux_UART_INSTANCE_CONNECT_CLIENT(
            chanMux_UART,
            tester2, chan
        )
    }
    configuration {
        // Assign badges to all three testers
        ChanMux_UART_CLIENT_ASSIGN_BADGES(
            tester1.chanMux_Rpc,
            tester2.chanMux_Rpc
        )
    }
}
```

### Using the Component\'s Interfaces in C

Client initialization example:

```c
#include <camkes.h>

// The following assignments are possible because camkes.h is included
// please note that the names like "chanMux_Rpc_write" are there in
// that way because of the naming composition from the ChanMux macros.
// The developer must take care of these in order to match the right
// external symbols provided by the camkes auto-generated code.

static const ChanMuxClientConfig_t chanMuxClientConfig = {
    .port   = CHANMUX_DATAPORT_ASSIGN(
                chanMux_chan_portRead,
                chanMux_chan_portWrite),
    .wait   = chanMux_chan_EventHasData_wait,
    .write  = chanMux_Rpc_write,
    .read   = chanMux_Rpc_read
};

OS_Error_t
ChanMuxTest_init(void)
{
    bool isSuccess = ChanMuxClient_ctor(
                        &testChanMuxClient,
                        &chanMuxClientConfig);
    if (!isSuccess)
    {
        Debug_LOG_ERROR("Failed to construct testChanMuxClient!");
        return -1;
    }
    return 0;
}
```
