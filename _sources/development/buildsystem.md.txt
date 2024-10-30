# Buildsystem

## Overview and Invocation

The TRENTOS build system uses CMake/ninja and is based on the
seL4/CAmkES build system. For certain parameters, e.g. the supported
platform or specific build variants, the seL4 and CAmkES documentation
must be consulted. The master build script is build-system.sh (residing
within the **`trentos/sdk`** folder), which supports an
out-of-source build for arbitrary OS projects. The script is designed to
be started from an arbitrary directory, there is no requirement that it
must be executed inside the SDK folder itself.

This script must be invoked as

```bash
trentos/sdk/build-system.sh
             <OS_PROJECT_DIR>
             <BUILD_PLATFORM>
             <BUILD_DIR>
             -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>
             ...
```

Where

- **`<SDK>`** is the path to the SDK location of the script, by
    default **`<sdk_root_directory>/sdk`**

- **`<OS_PROJECT_DIR>`** is the path to the OS project to build.

- **`<BUILD_PLATFORM>`** is the target platform (refer to the seL4
    build system for further details,
    <https://docs.sel4.systems/projects/buildsystem/>)

- **`<BUILD_DIR>`** is the folder where the build output will be
    created in, usually a sub-directory of the folder where the script
    is invoked in (i.e. the current working directory).

- **`-DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>`**
    is a CMake parameter specifying the type of the build. The build
    types differ in the following way:

|   Build type   |                    Description                   |                  Compiler flags                  |
|:--------------:|:------------------------------------------------:|:------------------------------------------------:|
| Debug          | Non-optimized build with debugging symbols.      | -g                                               |
| Release        | Fully optimized build without debugging symbols. | -O3 -DNDEBUG -ffunction-sections -fdata-sections |
| RelWithDebInfo | Optimized build with debugging symbols.          | -O2 -g -DNDEBUG                                  |
| MinSizeRel     | Size optimized build without debugging symbols.  | -Os -DNDEBUG -ffunction-sections -fdata-sections |

- any additional parameters will be passed to CMake.

The script **`build-system.sh`** internally invokes CMake with the file
**`<SDK>/CMakeLists.txt`**. The CMake build performs the following
steps:

- set up the build environment for seL4/CAmkES using
    **`<SDK>/sdk-sel4-camkes/helper.cmake`**
- build the seL4 microkernel
- build the CAmkES tools
- includes a custom file **`<OS_PROJECT_DIR>/CMakeLists.txt`** via the
    CMake function **`add_subdirectory()`** that defines the custom OS
    system to build. The system\'s build output will be in the folder
    **`<build-directory>/os_system`** then.

For the content of **`<OS_PROJECT_DIR>/CMakeLists.txt`** please refer to
the demo systems shipped with the SDK. The Hello World demo is a simple
system with just one component that prints **`"hello world"`** to the
console. The IoT demo is a more complex system that uses various TRENTOS
modules. Please refer to the respective demo documentation within the
TRENTOS Handbook for further details. A comprehensive explanation can
also be found in the CAmkES specific tutorials at
<https://docs.sel4.systems/Tutorials>.

## CMake Build System Internals

### Variable OS_SDK_PASSIVE_CMAKE

By default, the build process starts in **`<SDK>/CMakeLists.txt`** which
executes CMake code to set up things and additionally includes the
custom **`<OS_PROJECT_DIR>/CMakeLists.txt`**.
When **`OS_SDK_PASSIVE_CMAKE`** is set to
true, then **`<SDK>/CMakeLists.txt`** will not execute anything.
Instead, all of its functions and macros must be invoked explicitly by a
fully customized build process. This feature is used for building the
SDK host tools (**`cpt`**, **`proxy`** and **`rdgen`**). Please refer to
their **`CMakeLists.txt`** files for further details.

### C Standard

By default, C11 with GNU extensions is supported.

The CMake build system defines C_STANDARD as 11 which means that the
supported C standard is **ISO/IEC 9899:2011** with enabled **GNU
extensions**. Hence, the **`-std=gnu11`** option gets appended during
the build process to the gcc command line.

### C Standard Library

When declaring a component with **`DeclareCAmkESComponent()`** it will
implicitly include musl libc as the C standard library. musll ibc is
built from **`<SDK>/sdk-sel4-camkes/libs/musllibc`**.

### C++ support

The build system was extended to allow the mixing of C and C++ source
code files in a project.

```CMake
project(tests_hello_world CXX)

DeclareCAmkESComponent(
    hello_world_app
    INCLUDES
        # no include paths needed
    SOURCES
        components/SystemController/SystemController.c
        components/SystemController/main.cpp
    C_FLAGS
        -Wall
        -Werror
    CXX_FLAGS
        -Wall
        -Werror
        -fno-exceptions
        -fno-rtti
        -fno-threadsafe-statics
    LIBS
        os_core_api
)
```

### Limitations

- no standard library and standard template library is available at
    this moment
  - operators new, new[\], delete, delete[\] must be implemented
        by the used if needed (for example as wrappers for malloc from
        musllib)
- no exceptions support (**`-fno-exceptions`**)
- no run-time type information support (**`-fno-rtti`**)
- no thread safe statics (**`-fno-threadsafe-statics`**)
  - to enable the feature the user must implement the mutex
        functions and remove the CXX flag

### Function os_sdk_setup()

This is a convenience function that takes a header file as a parameter,
which holds a system-specific global configuration. The function\'s
parameters are:

```CMake
#  CONFIG_FILE <cfg_file>
#    config file, required when using certain components
#
#  CONFIG_PROJECT_NAME <name>
#    optional, create config project for config file. The project provides an
#    interface library that provides the config file's include path, so the
#    config file can be included in other files also.
```

It will set up the module-specific defines for configuration files

- **`DEBUG_CONFIG_H_FILE`**
- **`MEMORY_CONFIG_H_FILE`**
- **`OS_Logger_CONFIG_H_FILE`**

to point to this single configuration file. These will also set up the
include path for the CAmkES files. Please refer to the modules for a
detailed description of the configuration options.

As a further convenience, a lightweight CMake interface library can be
created automatically, that wraps the include path handling. For a usage
example where the file **`config/my_system_config.h`** also contains
definitions for a system controller component, the main CMake file of a
system could contain the following:

```CMake
os_sdk_setup(CONFIG_FILE "system_config.h" CONFIG_PROJECT "system_config")

project(my_system C)

DeclareCAmkESComponent(
    SystemController
    SOURCES
        components/SystemController/SystemController.c
    C_FLAGS
        -Wall
        -Werror
    LIBS
        my_system_config   # provides the include directory "config"
        os_core_api
)
```

In the corresponding **`components/SystemController/SystemController.c`** the
include path is set and the configuration file can be included with:

```c
#include "my_system_config.h"

...
```

In a more complex system where one global configuration file is not
desired, the content of the function **`os_sdk_setup()`** must be
implemented manually in the appropriate CMake files.

## Adjust the Log Output Verbosity

### For TRENTOS Libraries or Components

The TRENTOS log system can be configured with the following settings:

```c
// Debug_LOG_LEVEL_ASSERT  1
// Debug_LOG_LEVEL_FATAL   2
// Debug_LOG_LEVEL_ERROR   3
// Debug_LOG_LEVEL_WARNING 4
// Debug_LOG_LEVEL_INFO    5
// Debug_LOG_LEVEL_DEBUG   6
// Debug_LOG_LEVEL_TRACE   7
#define Debug_Config_LOG_LEVEL   Debug_LOG_LEVEL_INFO

// print log level of message as part of the message
#define Debug_Config_INCLUDE_LEVEL_IN_MSG

// print file and line number per message
#define Debug_Config_LOG_WITH_FILE_LINE

// Debug_ASSERT() and Debug_STATIC_ASSERT() use stdlib's assert.h functions
#define Debug_Config_STANDARD_ASSERT

// Debug_ASSERT_SELF(self) falls down to Debug_ASSERT(self != NULL)
#define Debug_Config_ASSERT_SELF_PTR

#include "LibDebug/Debug.h"
```

Only messages printed with **`Debug_LOG_xxx()`** where
**`xxx >= Debug_Config_LOG_LEVEL`** are actually compiled into the code.
If the log message is sent to the system log server, there is also an
extensive configuration mechanism there, that allows defining if the
messages compiled into the code are shown in the logs eventually.

### For seL4/CAmkES Libraries

The seL4/CAmkES libraries console output verbosity is controlled by the
CMake variable **`LibUtilsDefaultZfLogLevel`**, which will define
**`ZF_LOG_LEVEL`** used in
**`<SDK>/sdk-sel4-camkes/libs/sel4_util_libs/libutils/include/utils/zf_log.h`**.
The build system will set a default level 2 (**`ZF_LOG_DEBUG`**), that
is set up in the CMake file **`<SDK>/CMakeLists.txt`**. It can be
overwritten in a project\'s **`CMakeLists.txt`** file when using:

```c
// ZF_LOG_VERBOSE  1
// ZF_LOG_DEBUG    2
// ZF_LOG_INFO     3
// ZF_LOG_WARN     4
// ZF_LOG_ERROR    5
set(LibUtilsDefaultZfLogLevel 5 CACHE STRING "" FORCE)
```

## Running System Images in QEMU

The seL4/CAmkES build system creates a system image
**`<BUILD_DIR>/images/capdl-loader-image-<sel4_arch>-<sel4_plat>`**. The
resulting name is not always intuitive, it requires deep insight into
seL4. For example, the target **`zynq7000`** will result in the
name **`capdl-loader-image-arm-zynq7000`**, but for the target
**`rpi3`** it is **`capdl-loader-image-arm-bcm2837`**. To simplify
post-processing and usage, the TRENTOS build system will create a copy
of this image under the generic
name **`<BUILD_DIR>/images/os_image.elf`**. For compatibility with older
SDK versions, a copy **`<BUILD_DIR>/images/os_image.bin`** also exists
and some TRENTOS tools still use this image name. However, this name is
reserved for board specific binary system images created from the ELF
image in the future.

Note that native seL4/CAmkES tools and scripts use the
file **`<BUILD_DIR>/images/capdl-loader-image-<sel4_arch>-<sel4_plat>`**.
For example, the seL4/CAmkES build system provides a Python script that
can be used to run the QEMU simulation. It must be executed from within
the build output folder:

```shell
cd ${BUILD_DIR}
./simulate
```

and will use the image **`capdl-loader-image-arm-zynq7000`**. For
stand-alone systems like the Hello World demo, that do not use
networking or persistent storage, this is all that is required to run
the system.

Networking and persistent storage is implemented via the
[ChanMux](../components/chan-mux.md) component and a helper
[Proxy Application](../tools/proxy-application.md) that runs on the host system.
It connects to QEMU's virtual serial port and provides a storage backend based
on a file and networking capabilities via Linux TAP devices. The TRENTOS SDK
provides the script **`<SDK>/scripts/run_qemu.sh`**, which takes the system
image as a parameter and optionally a mode used to connect QEMU to the proxy
application. Please refer to the [Proxy Application](../tools/proxy-application.md)
documentation for more information.

```bash
# start QEMU and connect to proxy app via TCP
sdk/scripts/run_qemu.sh ${BUILD_DIR}/images/os_image.elf TCP
```

## ELF Files and Disassemblies for Components or System Images

Besides the ELF files for each component and the system image, the build
system will also use the script **`<SDK>/scripts/elf-dump.sh`** to
create a text file with symbol addresses and a disassembly of to support
debugging. This disassembly is in the corresponding **`*.lst`** file,
e.g. **`<BUILD_DIR>/images/os_image.elf.lst`** for the system image. For
any components, the ELF file
is **`<BUILD_DIR>/os_system/<inst-name>.instance.bin`** and
**`<BUILD_DIR>/os_system/<inst-name>.instance.bin.lst`**.
The seL4 kernel can be found in **`<BUILD_DIR>/kernel/kernel.elf`**
and **`<BUILD_DIR>/kernel/kernel.elf.lst`**. The ElfLoader can be found
in **`<BUILD_DIR>/elfloader/elfloader`**
and **`<BUILD_DIR>/elfloader/elfloader.lst`**.

## Static Code Analyzer

By default, the static code analyzer is deactivated when building the system
because this increases the build time. It can be enabled by adding the parameter
**`-DENABLE_LINT=ON`** when starting the build.

```shell
<SDK>/build-system.sh ... -DENABLE_LINT=ON
```

## Resources

- CMake: [https://cmake.org](https://cmake.org/)
- Ninja: [https://ninja-build.org](https://ninja-build.org/)
- seL4/CAmkES build system: <https://docs.sel4.systems/projects/buildsystem>
