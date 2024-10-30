# Storage_ChanMux

## Overview

This component makes a ChanMux storage channel available via the
**`if_OS_Storage`** interface. The most common use case is persisting data
on the host machine when TRENTOS runs in QEMU.
See [ChanMux](../components/chan-mux.md) component for more details.

## Usage

### Declaration of the Component in CMake

Use the following predefined macro in your project:

```CMake
Storage_ChanMux_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

### Instantiation and Configuration in CAmkES

An instance of Storage_ChanMux is connected to an instance of ChanMux,
thus ChanMux and a UART and must be instantiated as well. Details can be
seen in the example further down. Here we show only the macros specific
to the Storage_ChanMux component.

#### Declaring the Component

This component's CAmkES file must be included and then the component
can be defined.

```c
#include "Storage_ChanMux/Storage_ChanMux.camkes"
Storage_ChanMux_COMPONENT_DEFINE(<NameOfComponent>)
```

#### Instantiating and Connecting the Component

An instance of the Storage_ChanMux is connected to another component
instance via **`if_OS_Storage`**:

```c
component <NameOfComponent> <nameOfInstance>;
component <SomeComponent> <someComponentInstance>;

Storage_ChanMux_INSTANCE_CONNECT_CLIENT(
    <nameOfInstance>,
    <someComponentInstance>.<storage_rpc>, <someComponentInstance>.<storage_port>
)
```

## Example

### Instantiation of the Component in CMake

We set up the Storage_ChanMux in the CMake file:

```CMake
Storage_ChanMux_DeclareCAmkESComponent(
    Storage_ChanMux
)
```

### Instantiation and Configuration in CAmkES

In the following example we show how to set up a complete system that
uses Storage_ChanMux with a [ChanMux](../components/chan-mux.md) UART component:

```c
#include "ChanMux/ChanMux_UART.camkes"

ChanMux_UART_COMPONENT_DEFINE(
    ChanMux_UART,
    storage_ChanMux, chan
)

#include "Storage_ChanMux/Storage_ChanMux.camkes"

Storage_ChanMux_COMPONENT_DEFINE(
    Storage_ChanMux
)

assembly {
    composition {

        // Instantiate ChanMux_UART and a UART
        component ChanMux_UART          chanMux_UART;
        component UART_0                uart;

        // Connect ChanMux to UART
        ChanMux_UART_INSTANCE_CONNECT(
            chanMux_UART,
            uart
        )

        // Instantiate Storage_ChanMux
        component Storage_ChanMux       storage_ChanMux;

        // Connect Storage_ChanMux to ChanMux_UART instance
        ChanMux_UART_INSTANCE_CONNECT_CLIENT(
            chanMux_UART,
            storage_ChanMux, chan
        )

        // Instantiate an application
        component ExampleApplication    app;

        // Connect application instance to ChanMux_Storage instance
        Storage_ChanMux_INSTANCE_CONNECT_CLIENT(
            storage_ChanMux,
            app.storage_rpc, app.storage_port
        )
    }
    configuration {
        // Assign badge to Storage_ChanMux's RPC endpoint with ChanMux_UART
        ChanMux_UART_CLIENT_ASSIGN_BADGES(
            storage_ChanMux.chanMux_Rpc
        )
    }
}
```

Please note that currently the Storage_ChanMux's RPC endpoint is called
**`chanMux_Rpc`** and this name cannot be changed.

### Using the Component's Interfaces in C

In this example we use the client's **`if_OS_Storage`** interface directly to
write/read to the Storage_ChanMux instance.

```c
// For the CAmkES generated glue code
#include <camkes.h>

// For wrapped access to the interface
#include <OS_Dataport.h>
#include <if_OS_Storage.h>

static const if_OS_Storage_t storage =
    IF_OS_STORAGE_ASSIGN(
        storage_rpc,
        storage_port);

static const OS_Dataport_t port =
    OS_DATAPORT_ASSIGN(
        storage_port);

static const char testData[] = "Foo";
#define TEST_DATA_SZ    (sizeof(testData) / sizeof(*testData))

int run() {
    const size_t storageOffset = 10;
    size_t result;

    memcpy(OS_Dataport_getBuf(port), testData, TEST_DATA_SZ);

    if ((OS_SUCCESS != storage.write(storageOffset, TEST_DATA_SZ, &result))
        || (TEST_DATA_SZ != result))
    {
        Debug_LOG_ERROR("write() failed.");
        return;
    }

    if ((OS_SUCCESS != storage.read(storageOffset, TEST_DATA_SZ, &result))
        || (TEST_DATA_SZ != result))
    {
        Debug_LOG_ERROR("read() failed.");
        return;
    }

    if ((OS_SUCCESS != storage.erase(storageOffset, TEST_DATA_SZ, &result))
        || (TEST_DATA_SZ != result))
    {
        Debug_LOG_ERROR("erase() failed.");
        return;
    }

    if (OS_SUCCESS != storage.getSize(&result))
    {
        Debug_LOG_ERROR("getSize() failed.");
        return;
    }

    Debug_LOG_INFO("storage size is %zu", result);
}
```
