# RamDisk

## Overview

The RamDisk is a general-purpose implementation of a component that offers the
**`if_OS_Storage`** interface and maps all I/O functionality to an internal buffer
in RAM.

By default, the RamDisk storage is empty (i.e. filled with zeros) when
initialized. However, it can also be provided with an image file (during compile
time) which allows to pre-load it with data, e.g., with a file system.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The RamDisk needs to be instantiated in the CMake file. Besides the component
name, an additional parameter with the system configuration must be given.
The parameter with the image file is optional.

```CMake
RamDisk_DeclareCAmkESComponent(
    <NameOfComponent>
    IMAGE
        <ImageFile>
)
```

The second parameter is optional and allows linking the RamDisk with a disk
image. This image is generated with the
[RamDisk Generator Tool](../tools/ram-disk-generator-tool.md) (**`rdgen`**) and
will be loaded by the RamDisk into its memory upon startup. This way, the
RamDisk can be pre-provisioned with some data. In general it is a C source file
that consists of the RAMDISK_IMAGE array and the RAMDISK_IMAGE_SIZE variable,
however a detailed description of how this tool can be used can be found in the
respective section of this handbook.

### Instantiation and Configuration in CAmkES

The component is easy to set up with just one client and storage size to be
configured.

#### Declaring the Component

The component is declared as follows:

```c
#include "RamDisk/RamDisk.camkes"
RamDisk_COMPONENT_DEFINE(<NameOfComponent>)
```

#### Instantiating and Connecting the Component

The component is instantiated and connected to a user via its
**`if_OS_Storage`** interface as follows:

```c
assembly {
    composition {
        component <NameOfComponent>   <nameOfInstance>;

        RamDisk_INSTANCE_CONNECT_CLIENT(
            <nameOfInstance>,
            <client>.<storage_rpc>, <client>.<storage_port>
        )
    }
    ...
}
```

#### Configuring the Instance

To configure the size of the RamDisk (i.e., the size of its internal,
static buffer), the CAmkES configuration attribute must be set. This
attribute would typically be set as part of the system configuration:

```c
// main.camkes
assembly {
    composition {
        ...
    }

    configuration {
        <nameOfInstance>.storage_size = <ramDiskSize>;
    }
}
```

There is no hard limit to the RAM disk size besides the target system's
architectural limits. On 32-bit systems, no more than 4 GiB RAM can be
addressed, but some memory is also required for the driver itself. In
the end, the amount of memory must fit into the target systems' RAM
together with the rest of the system. The CapDL loader might fail
setting up the system if it exceeds the available memory.

## Example

In the following example, we instantiate the RamDisk with a size of 5
MiB and then use it with the file system.

### Instantiation of the Component in CMake

In the CMake file we set up a wrapper library for the RamDisk, so it
finds the config and also includes the RamDisk into the build:

```c
// Interface library so RamDisk can find the system_config.h
project(system_config C)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

...

// Declare the component
RamDisk_DeclareCAmkESComponent(
    MyRamDisk
)
```

### Configuration of the Component in the 'main.camkes' File

In the **`main.camkes`** we set the size of the RamDisk's internal
buffer to 5 MiB.

```c
configuration {
    MyRamDisk.storage_size = 5 * 1024 * 1024; // 5 MiB
}
```

### Instantiation and Configuration in CAmkES

In the main CAmkES composition, we instantiate the RamDisk and connect
it to its single client.

#### Declaring the Component

The component is declared as follows:

```c
#include "RamDisk/RamDisk.camkes"
RamDisk_COMPONENT_DEFINE(MyRamDisk)
```

#### Instantiating and Connecting the Component

The component is instantiated and connected to a user via its
**`if_OS_Storage`** interface:

```c
// Instantiate RamDisk
component MyRamDisk myRamDisk;

// Connect interface PROVIDED by RamDisk
RamDisk_INSTANCE_CONNECT_CLIENT(
    myRamDisk,
    ramDiskUser.ramDiskStorage_rpc, ramDiskUser.ramDiskStorage_port
)
```

### Using the Component's Interfaces in C

The RamDisk can be used by including the **`camkes.h`** header and calling the
RPC interface in the following way:

```c
// For the CAmkES generated interface
#include <camkes.h>

// For wrapped access to the interface
#include <OS_Dataport.h>
#include <if_OS_Storage.h>

#include "lib_macros/Check.h"

static const if_OS_Storage_t storage =
    IF_OS_STORAGE_ASSIGN(
        myRamDisk_rpc,
        myRamDisk_port);

static const OS_Dataport_t port =
    OS_DATAPORT_ASSIGN(
        myRamDisk_port);

...

int run() {
    const off_t   desiredOffset = 13;
    const size_t  dataSz        = 42;
    const uint8_t writePattern  = 0xA5;

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
