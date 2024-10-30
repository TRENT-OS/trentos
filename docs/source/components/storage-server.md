# StorageServer

## Overview

The StorageServer is a component that enables multiple client components
to access a storage device.

### Concepts

TRENTOS has a very generic concept of "storage". Access to storage is
defined by function pointers which can perform read/write/etc.
operations on said storage. It is up to the provider of a storage
interface to provide its implementation, which can be generic (e.g. a
RamDisk) or platform-specific (e.g. an SPI flash driver).

### Implementation

In order to allow multiple components to access a single "piece of
storage", storage is divided into different segments which can then be
accessed by the clients of the StorageServer.

For each client interface an offset/size pair is configured, which
limits the access for this client to the storage address range *offset*
to *offset+size-1*. On the interface from the clients to the
StorageServer the visible address range is *0* to *size-1*. The Storage
server performs the address translation internally.

**Info:** It is possible to set the offset values such that client
storage spaces overlap. However, this is not advisable if the
StorageServer\'s clients do not implement a mechanism to prevent race
conditions when simultaneously using the same underlying storage.

The StorageServer implements the **`if_OS_Storage`** interface for its
clients and uses the same interface for its own access to the underlying
storage.

The StorageServer supports up to 8 parallel clients.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The StorageServer can be declared via a simple macro in the
**`CMakeLists.txt`** file:

```CMake
StorageServer_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

### Instantiation and Configuration in CAmkES

In order to wire the StorageServer to a set of clients, it has to be
declared, instantiated, connected and configured in the main CAmkES
composition of the system.

#### Defining the Component

The component is simply declared by using its respective macro:

```c
#include "StorageServer/camkes/StorageServer.camkes"
StorageServer_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

#### Instantiating and Connecting the Component

The following macro allows to create an instance of the component and
connect it to the storage provider (e.g., RamDisk or Flash):

```c
component   <NameOfComponent> <nameOfInstance>;

StorageServer_INSTANCE_CONNECT(
    <nameOfInstance>
    <storage>.<storage_rpc>, <storage>.<storage_port>
)
```

The clients have to use the **`if_OS_Storage`** interface and provide a
dataport for transactions with the StorageServer.  The following macro
accepts a list of clients, as long as they are given in pairs with their
RPC interface and dataport:

```c
StorageServer_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>
    <client1>.<nameOfInterface>, <client1>.<nameOfDataport>,
    <client2>.<nameOfInterface>, <client2>.<nameOfDataport>,
    ...
)
```

#### Configuring the Instance

Every instance of the StorageServer has to be configured. For every
client listed above, we need to pass two values:

- **`offset:`** indicates where (from the beginning of the underlying
    storage) the storage of that client starts (in bytes)
- **`size:`** this parameter indicates how big the storage is (in bytes)

The following macro can be used:

```c
StorageServer_INSTANCE_CONFIGURE_CLIENTS(
    <nameOfInstance>,
    <offset1>, <size1>,
    <offset2>, <size2>,
    ...
)
```

Please note that the order of the configuration here must correspond to
the order in which the clients are listed in
the **`StorageServer_INSTANCE_CONNECT_CLIENTS()`** macro.

#### Assigning Clients\' Badges

The StorageServer uses the "badge" assigned by seL4 to each client\'s
RPC endpoint to map the client to its respective storage configuration;
badges can be assigned via this macro:

```c
StorageServer_CLIENT_ASSIGN_BADGES(
    <client1>.<nameOfInterface>,
    <client2>.<nameOfInterface>,
    ...
)
```

Please note that the order of clients needs to correspond to the order
**`StorageServer_INSTANCE_CONFIGURE_CLIENTS()`** and
**`StorageServer_INSTANCE_CONNECT_CLIENTS()`** macros. The StorageServer can
handle up to 8 clients.

## Example

Here is an example configuration of two clients using the StorageServer
to partition a RamDisk of 2 MiB into two storages of 1 MiB for each
client.

### Instantiation of the Component in CMake

The component is added to the build as MyStorageServer:

```CMake
StorageServer_DeclareCAmkESComponent(
    MyStorageServer
)
```

### Instantiation and Configuration in CAmkES

#### Defining the Component

The component is declared as MyStorageServer in the main CAmkES file:

```c
#include "StorageServer/camkes/StorageServer.camkes"
StorageServer_COMPONENT_DEFINE(
    MyStorageServer
)
```

#### Instantiating and Connecting the Component

The following instantiates
a [RamDisk](ram-disk.md) component, the StorageServer instance and connects them
together.

```c
// Instantiate RamDisk for storage and a StorageServer
component RamDisk           storage;
component MyStorageServer   myStorageServer;
// Instantiate two clients
component Client            client1;
component Client            client2;

// Connect interfaces USED by StorageServer
StorageServer_INSTANCE_CONNECT(
    myStorageServer,
    storage.storage_rpc,    storage.storage_port
)
// Connect interfaces PROVIDED by StorageServer
StorageServer_INSTANCE_CONNECT_CLIENTS(
    myStorageServer,
    client1.storage_rpc, client1.storage_port,
    client2.storage_rpc, client2.storage_port
)
```

Please note that in the beginning, we connect the StorageServer to a RamDisk via
the **`if_OS_Storage`** interface, whereas the second macro connects two clients
to the StorageServer, again via an instance of **`if_OS_Storage`** (and a
dataport).

#### Configuring the Instance

Here we see that for each entry in
the **`StorageServer_INSTANCE_CONNECT_CLIENTS()`** macro, we have a matching
line which contains an offset and a size value. As we can see, each
client gets 1 MiB of storage, but the storage of the second client
begins at the end of the first client's storage.

```c
StorageServer_INSTANCE_CONFIGURE_CLIENTS(
    myStorageServer,
    0,         1024*1024,
    1024*1024, 1024*1024
)
```

#### Assigning Clients' Badges

We assign the badges to the client's RPC endpoints via this macro:

```c
StorageServer_CLIENT_ASSIGN_BADGES(
    client1.myStorageServer_rpc,
    client2.myStorageServer_rpc
)
```

### Using the Component's Interfaces in C

The StorageServer can be used by including the **`camkes.h`** header and calling
the RPC interface in the following way:

```c
// For the CAmkES generated interface
#include <camkes.h>

// For wrapped access to the interface
#include <OS_Dataport.h>
#include <if_OS_Storage.h>

static const if_OS_Storage_t storage =
    IF_OS_STORAGE_ASSIGN(
        myStorageServer_rpc,
        myStorageServer_port);

static const OS_Dataport_t port =
    OS_DATAPORT_ASSIGN(
        myStorageServer_port);

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
