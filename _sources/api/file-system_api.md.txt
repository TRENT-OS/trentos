# FileSystem API

## Overview

The FileSystem API supports the creation of file systems of various
types and typical operations (read, write) on files.

### Concepts

The FileSystem API provides access to typical file system operations
based on an abstract concept of "storage". A piece of storage is
effectively defined by a set of function pointers that can be used from
within the FileSystem layer for performing read/write/erase operations.

In TRENTOS, any component that provides the **`if_OS_Storage`** interface can
be used through the FileSystem API.

### Implementation

The file system library is effectively a wrapper around third-party file
system implementations. Currently, the FileSystem API implementation
supports the following:

- **FAT:** implementation of a DOS/Windows compatible FAT file system
    (uses the FatFs library, see <http://elm-chan.org/fsw/ff/00index_e.html>)
- **SPIFFS:**  a file system intended for SPI NOR flash devices on
    embedded targets (uses the spiffs library, see <https://github.com/pellepl/spiffs>)
- **LITTLEFS**: a filesystem designed to handle random power failures
    (uses the LittleFS library, see <https://github.com/littlefs-project/littlefs>)

At this point, the API knows how to format/mount a "piece of storage" and
perform file operations on it.  Directories or folders are not supported.

Please note that FAT was never designed to be used on FLASH memories and
should not be used for such purposes.

Each file system implementation can have specific parameters set (i.e.,
in the case of FAT to configure for the underlying \"disk geometry\").
The FileSystem API implementation currently uses default values for
these parameters, which can be overridden by passing them explicitly as
part of the configuration.

## Usage

This is how the API can be instantiated in the system.

### Declaration of API Library in CMake

The FileSystem API can be pulled into the build process by referencing
the following build target:

- **os_filesystem:** Generic build target to pull in all available
    file system implementations.

## Example

To use the filesystem in a project, it must be linked as library via
CMake. Additionally, the FileSystem API requires access to an underlying
storage layer (e.g., a RamDisk), which can be provided by connecting
the **`if_OS_Storage`** interface to a component implementing it. Please
see the respective chapters on how to set up and connect the different
storage options (e.g., RamDisk, SPI_Flash, etc.).

### Instantiation of API in CMake

To use the FileSystem API it needs to be pulled into the build of a
component by adding the above-mentioned build targets.

```CMake
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_filesystem
)
```

### Using the API in C

In the following, a FileSystem API instance is configured to use the
storage interface of the [RamDisk](../components/ram-disk.md) component
(**`storage_rpc`** for the interface, **`storage_port`** for a shared dataport
between the [RamDisk](../components/ram-disk.md) component and the file system
library).

After instantiating the library, it is then used to create and mount a FAT
partition with maximum size (as reported by the RamDisk). Finally, with the now
existing file system, some basic file I/O operations are performed, before the
file system is unmounted again.

The code is slightly abbreviated for clarity.

```c
// Include FileSystem API
#include "OS_FileSystem.h"

// For access to storage component
#include <camkes.h>

static OS_FileSystem_Config_t cfg =
{
    .type = OS_FileSystem_Type_FATFS,
    .storage = IF_OS_STORAGE_ASSIGN(
        storage_rpc,
        storage_port),
};
...

int run()
{
    OS_FileSystem_Handle_t hFile;
    OS_FileSystemFile_Handle_t hFs;
    uint8_t fileData[16];

    // Init file system
    OS_FileSystem_init(&hFs, &cfg);
    // Format it and mount it
    OS_FileSystem_format(hFs);
    OS_FileSystem_mount(hFs);

    // Open file
    OS_FileSystemFile_open(
        hFs,
        &hFile,
        "testfile",
        OS_FileSystem_OpenMode_RDWR,
        OS_FileSystem_OpenFlags_CREATE);
    // Write file
    OS_FileSystemFile_write(hFs, hFile, 0, sizeof(fileData), fileData);
    // Close file
    OS_FileSystemFile_close(hFs, hFile);

    // Clean up
    OS_FileSystem_unmount(hFs);
    OS_FileSystem_free(hFs);
}
```
