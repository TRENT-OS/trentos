# UART

## Overview

The UART component implements a generic UART driver that can be used on
various platforms.

### Concepts

The component wraps the CAmkES details for creating a UART driver and
connecting it to the hardware.

### Architecture

```{mermaid}
flowchart BT
    %% Components and Databases
    subgraph App["App"]
      app_lib_fifo_dataport["FIFO dataport lib"]
    end
    output_buffer{{"output buffer"}}
    input_buffer{{"input buffer"}}
    subgraph UART["UART Driver"]
      uart_lib_fifo_dataport["FIFO dataport lib"]
    end
    HW("UART Hardware")

    %% Connections
    App -. "if_OS_BlockingWrite" .-> UART
    app_lib_fifo_dataport <--"EventDataAvailable"--> UART
    app_lib_fifo_dataport --> output_buffer
    input_buffer --> app_lib_fifo_dataport
    uart_lib_fifo_dataport --> input_buffer
    output_buffer --> UART
    UART ..-> HW
```

### Implementation

The component uses the seL4 platform library code for the
platform-specific low-level hardware access. The UART is used in raw
mode, no additional characters are added or removed from the data
stream. Especially, no CR/LF line break conversion is applied, which is
otherwise common when using the UART as a console.

The purpose of the UART driver in the current implementation is for
binary data transfer, and it is currently not intended for console
usage. For log output, the LogServer component should be used.

#### Interface

The UART component exposes one CAmkES RPC interface
**`if_OS_BlockingWrite`** and two data ports. When calling the RPC **`write()`**,
the driver will pick the data from the output dataport and send it out on the
UART. The input dataport is organized as a FIFO and must be accessed via
**`lib_io/FifoDataport.h`** from TRENTOS libraries.

```c
procedure if_OS_BlockingWrite
{
   // The actual data is in the dataport, the function blocks until the last
   // byte is sent by the underlying driver.
   void write(size_t len);
};
```

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

For simplicity and portability it is recommended to declare all
available UARTs with the following function:

```CMake
DeclareCAmkESComponents_for_UARTs()
```

However, if a more fine tuned configuration is required, the UART
component can be declared as follows:

```CMake
UART_DeclareCAmkESComponent(
    <NameOfComponent>
    <dtbName>
)
```

Note that **`<dtbName>`** must be taken from the platform's device tree. You can
refer to component's **`plat/<platform>/UART_plat_config.camkes`** for the
existing configurations.

### Instantiation and Configuration in CAmkES

Here we show how to instantiate and connect the component.

#### Declaring the Component

For simplicity and portability all available UARTs are declared
automatically when the component's CAmkES file is included:

```c
#include "UART/Uart.camkes"
```

### Instantiating and Connecting the Component

To instantiate the UART and connect it to a client (via
**`if_OS_BlockingWrite`**, two dataports and an event) the following can be
used:

```c
component UART_<id> <nameOfInstance>;

UART_INSTANCE_CONNECT_CLIENT(
    UART_<id>,
    <client>.<uart_rpc>,
    <client>.<uart_input_port>,
    <client>.<uart_output_port>,
    <client>.<uart_event>
)
```

Since UART component type definitions are pre-defined, their names are
specified with the index suffix (e.g. **`UART_0`**,  **`UART_1`**).
Please refer to the component's
**`plat/<platform>/UART_plat_config.camkes`**  file for the list of the
available interfaces.

For example, for the **`zynq7000`** platform that file would look like:

```c
UART_COMPONENT_DEFINE(UART_0, {"path":"/amba/serial@e0000000"})
UART_COMPONENT_DEFINE(UART_1, {"path":"/amba/serial@e0001000"})
```

**`UART_COMPONENT_DEFINE()`** takes two mandatory and one optional parameter.
Therefore it can either be used as

```c
UART_COMPONENT_DEFINE(component_type_name, device_tree_name)
```

or

```c
UART_COMPONENT_DEFINE(component_type_name, device_tree_name, dataport_size)
```

The dataport size must be a multiple of 4096 bytes, because shared memory is
build on sharing MMU pages, which have 4 KiB granularity. If no dataport size is
given, the size is 4 KiB.

## Example

In TRENTOS the UART is mostly used through the [ChanMux](chan-mux.md) component,
which can map different input/output streams to a single UART instance.

In this example we show how to use the UART directly.

### Instantiation of the Component in CMake

The UART can simply be added to the build like this:

```CMake
DeclareCAmkESComponents_for_UARTs()
```

### Instantiation and Configuration in CAmkES

In this example, the component is connected to a client who uses the
UART directly.

#### Declaring the Component

The UART can be declared as follows:

```c
#include "UART/UART.camkes"
```

#### Instantiating and Connecting the Component

To instantiate and connect the UART the following can be used:

```c
// Instantiate the UART driver
component  UART_0   uart;
// Instantiate a client
component  Client   client;

// Connect UART to client
UART_INSTANCE_CONNECT_CLIENT(
    uart,
    client.uart_rpc,
    client.uart_input_port,
    client.uart_output_port,
    client.uart_event
)
```

### Using the Component's Interfaces in C

In the following example we use the UART's RPC interface directly to write some
data to its output dataport.

```c
// For the CAmkES generated interface
#include <camkes.h>

// To use the dataport via proper macros
#include <OS_Dataport.h>

static const OS_Dataport_t port =
    OS_DATAPORT_ASSIGN(
        uart_output_port);

static const char testData[] = "Foo";

# TEST_DATA_SZ includes the terminating NULL char.
#define TEST_DATA_SZ  sizeof(testData)

...

int run() {
    ...

    void *dataport_buffer = OS_Dataport_getBuf(port);

    // Write data to dataport's buffer and write() from the RPC interface
    memcpy(dataport_buffer, testData, TEST_DATA_SZ);
    uart_rpc_write(TEST_DATA_SZ);

    ...
}
```

### Portability Improvements

Since all the platform's UARTs are available for the end application, there is a
need for the platform specific connection of the clients to the specific UARTs.

This can be achieved by introducing in the application a platform specific
configuration header and including it in the **`main.camkes`** file:

```c
#include "plat_system_config.h"

assembly {
    composition {
        component UART_FOO uartFoo;
        component UART_BAR uartBar;
    }
}
```

The platform's header files are located in platform-specific subfolder as
follows:

```
<App>/plat/
├── <PlatformA>
│   └── plat_system_config.h
└── <PlatformB>
    └── plat_system_config.h
```

Exemplary content of those headers is presented below:

```c
// UARTs for Platform A
#define UART_FOO UART_0
#define UART_BAR UART_1
```

```c
// UARTs for Platform B
#define UART_FOO UART_3
#define UART_BAR UART_4
```
