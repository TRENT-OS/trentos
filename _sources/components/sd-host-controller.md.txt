# SdHostController

## Overview

The SdHostController is a driver that implements **`if_OS_Storage`** and
allows accessing the SD card peripheral.

### Implementation

This component will automatically initiate the SD card hardware
peripheral during the initialization phase so that the card can be
accessed via the blocking RPC calls with data being exchanged via the
dedicated data port.

**Warning:** Please note that the driver currently assumes that the SD
card is inserted during the entire power cycle, and does not support SD
card removal/insertion events.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

For declaring this component in CMake, pass only the type name of the
component as below:

```CMake
SdHostController_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

### Instantiation and Configuration in CAmkES

The component requires only the connection with the client and the
hardware configuration to be set properly.

#### Declaring the Component

The SdHostController consists of two parts, that have to be declared
separately:

- the host controller driver component
- the hardware component

Both need to be declared like this:

```c
#include "SdHostController/SdHostController.camkes"
SdHostController_COMPONENT_DEFINE(<NameOfComponent>)
SdHostController_HW_COMPONENT_DEFINE(<NameOfHWComponent>)
```

Please note that the name of the driver component must match the name
used in the CMake file.

#### Instantiating and Connecting the Component

The components are instantiated and connected to each other as follows:

```
component   <NameOfComponent>     <nameOfInstance>;
component   <NameOfHWComponent>   <nameOfHWInstance>;

SdHostController_INSTANCE_CONNECT(
    <nameOfInstance>,
    <nameOfHWInstance>
)
```

Once the component is set up, it can be connected to a single client
which uses the if_OS_Storage interface. The **`SdHostController`**
provides the CAmkES endpoints **`<nameOfInstance>.storage_rpc`** and
**`<nameOfInstance>.storage_port`** to connect to a client.\
Alternatively one can use the following macro to connect to a client:

```c
SdHostController_INSTANCE_CONNECT_CLIENT(
    <nameOfInstance>,
    <client>.<storage_rpc>, <client>.<storage_port>
)
```

#### Configuring the Instance

The following two macros need to be called in the **`configuration`**
section of a CAmkES **`assembly`**. They will provide a default
configuration on the selected build platform.

```c
SdHostController_INSTANCE_CONFIGURE(
    <nameOfInstance>
)
SdHostController_HW_INSTANCE_CONFIGURE(
    <nameOfHWInstance>
)
```

## Example

In the following example, we instantiate the SdHostController for the
default peripheral address and IRQ.

### Instantiation of the Component in CMake

SdHostController has been chosen as the component's name:

```c
SdHostController_DeclareCAmkESComponent(
    SdHostController
)
```

### Instantiation and Configuration of the Component in CAmkES

In the main CAmkES composition, we instantiate the SdHostController,
connect it to its single client, and configure the parameters of the
peripheral correctly.

#### Declaring the Component in the Main CAmkES File

Following what was specified in the CMake file, we declare the driver
component and the HW part:

```c
#include "SdHostController/SdHostController.camkes"
SdHostController_COMPONENT_DEFINE(SdHostController);
SdHostController_HW_COMPONENT_DEFINE(SdHostController_HW);
```

#### Instantiating and Connecting the Component in the Main CAmkES File

In the example below the SdHostController is instantiated and connected to a
client:

```c
// Instantiate driver and HW component
component   SdHostController_HW sdhcHw;
component   SdHostController    sdhc;
// Instantiate client
component Client                client;

// Connect interface USED by driver
SdHostController_INSTANCE_CONNECT(
    sdhc,
    sdhcHw
)
// Connect interface PROVIDED by driver
SdHostController_INSTANCE_CONNECT_CLIENT(
    sdhc,
    client.storage_rpc, client.storage_port
)
```

#### Configuring the Instance in the Main CAmkES File

The desired peripheral port can be configured in two different ways:
either by using the platform-specific default configuration or by
directly selecting the peripheral port from the CAmkES file of the
system. The default configurations for the different platforms can be
found in the sources of the SdHostController component and are also
documented in the
[PlatformSupport](../platform-support/using-sd-card-with-trentos.md) chapter.

The following example will configure the instance to the
platform-specific default:

```c
// Configure driver/HW to use the platform-specific default configuration
SdHostController_INSTANCE_CONFIGURE(
    sdhc
)
SdHostController_HW_INSTANCE_CONFIGURE(
    sdhcHw
)
```

Selecting a specific peripheral port can be done using the following
macro, that will not only require the instance name but also the port
index of the peripheral that should be configured:

```c
// Configure driver/HW to use SDHC4
SdHostController_INSTANCE_CONFIGURE_BY_INDEX(
    sdhc,
    4
)
SdHostController_HW_INSTANCE_CONFIGURE_BY_INDEX(
    sdhcHw,
    4
)
```

### Using the Component's Interfaces in C

The **`if_OS_Storage`** interface is used to provide raw access to the SD card. This
is demonstrated in the example below, in which the card is
accessed directly without any FileSystem or
[StorageServer](../components/storage-server.md) component in between.

```c
// For the CAmkES generated interface
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

...

int run() {
    const size_t    blockSz         = storage.getBlockSize();
    const size_t    nBlocks         = 3;
    const size_t    dataSz          = blockSz * nBlocks;
    const off_t     desiredOffset   = 1 * blockSize;
    const uint8_t   writePattern    = 0xA5;

    void* const dataPort = OS_Dataport_getBuf(port);

    CHECK_PTR_NOT_NULL(dataPort);

    // Verify in the production code that there is no data port overflow.
    memset(dataPort, writePattern, dataSz);

    size_t bytesWritten = 0U;
    if(OS_SUCCESS != storage.write(
                            desiredOffset,
                            dataSz,
                            &bytesWritten))
    {
        // Handle the write error.
    }

    if(dataSz != bytesWritten)
    {
        // Handle the write error.
    }

    // Clearing the data port for sanity.
    memset(dataPort, 0xFF, dataSz);

    size_t bytesRead = 0U;
    if(OS_SUCCESS != storage.read(
                            desiredOffset,
                            dataSz,
                            &bytesRead))
    {
        // Handle the read error.
    }

    if(dataSz != bytesRead)
    {
        // Handle the read error.
    }

    // Verifying the read content:
    for(size_t i = 0; i < dataSz; ++i)
    {
        if(writePattern != dataPort[i])
        {
            // Shall never happen!
        }
    }

    ...
}
```

Please note that data chunks can only be written and read from the
medium in block size multiples. In addition, offsets also need to
conform to this rule.
