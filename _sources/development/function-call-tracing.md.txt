# Function Call Tracing

## Function Instrumentation

The **`gcc`** option **`-finstrument_functions`** allows defining two
functions that will be injected at the entry and at the exit of
functions compiled with that compiler option.

One application of this compiler option is a function call tracing made
with standard output.

### Debug Functions

TRENTOS provides basic implementations of the instrumentation functions
**`__cyg_profile_func_enter()`** and **`__cyg_profile_func_exit()`** in
**`instrument_functions.c`**. When including **`lib_debug`** those functions
are included automatically into the project.

The functions print an entry marker or an exit marker together with the
function address when entering or exiting functions compiled with the
compiler option.

If a user wants to provide its own implementations, the default
implementations can be deactivated in the configuration of
**`lib_debug`** by defining **`LIB_DEBUG_INSTRUMENT_FUNCTIONS 0`**.

### Compiler Option

Usually one wants to debug certain components or libraries and the
**`gcc`** option **`-finstrument_functions`** has to be set for the
corresponding source files.

**WARNING:** It is not possible to set the compiler option globally to
debug the full system stack because the used standard output via UART is
not always available and its throughput is limited.

The activation of the compiler option for a component is straightforward
by setting the **`COMPILE_OPTIONS`** property for all relevant source
files. For a library of type **`INTERFACE`** it is easily possible as
well without changing the library because it gets compiled together with
the component. Otherwise, a modification of the library would be
necessary.

Below is an example of how the source files of a component and the
source files of a specific library can be compiled with the compiler
option. In the example, the source files of the component are available
in a CMake variable and the source files of the **`INTERFACE`** library are
collected into a CMake variable. For each source file, the **`COMPILE_OPTIONS`**
property is modified to have the **`-finstrument_functions`** flag.

```CMake
DeclareCAmkESComponent(
    MyComponent
    SOURCES
        ${MY_COMPONENT_SOURCES}
    LIBS
        lib_debug # required for instrument_functions.c
        my_library
)

# Set gcc option for component source files
set_source_files_properties(${MY_COMPONENT_SOURCES}
    PROPERTIES
        COMPILE_OPTIONS "-finstrument-functions"
)

# Get source files of INTERFACE library "my_library"
get_target_property(MY_LIBRARY_SOURCES
    my_library INTERFACE_SOURCES
)
# Set gcc option for library source files
set_source_files_properties(${MY_LIBRARY_SOURCES}
    PROPERTIES
        COMPILE_OPTIONS "-finstrument-functions"
)
```

## Backtrace Function Calls

The **`gcc`** option **`-finstrument_functions`** is a really powerful
feature. Its only drawback is that it is not possible to know the name
of the functions at runtime. The arguments
of **`__cyg_profile_func_enter()`** and **`__cyg_profile_func_exit()`** are
just the addresses of the current function and its caller.

TRENTOS offers the script **`sdk/scripts/debug/backtrace.py`** to have a pretty and
helpful format with function names.

### Backtrace Script

The script will display the function names with their nesting level
together with the usual standard output logs.

Therefore the script requires the standard output of the system to be
redirected to a file. If the system is for example run in QEMU, the QEMU
output has to be redirected to a file like **`qemu_stdout.txt`** . For
the demos in the SDK, only the error output is typically redirected to a
file **`qemu_stderr.txt`** through the **`run_demo.sh`** scripts and
they, therefore, need to be adapted to use the script.

In addition to the standard output file of the system, the script is
called with the symbol file created when building the system. So the
script is able to convert the printed addresses into the function names.

Since the script opens the standard output file in a non-blocking way,
it is even possible to use it while the system is running. Then the optional
**`timeout`** parameter should be considered to set an appropriate timeout the
script keeps waiting for new input.

#### Usage

The usage of the script is given in its Python documentation:

```Python
Backtraces -finstrument-functions markers with function hex addresses into
function names.

The script expects as arguments:
    --stdout_file:  An output file of a system (either still running or not)
                    containing the markers produced by enabling the gcc flag
                    -fintstument-functions
    --symbols_file: A .lst map file containing the symbols supposed to match the
                    hex addresses
    --timeout:      A timeout (in secs) parameter to exit the script when
                    waiting for new input. Default value is 0.

It will print out the same content of stdout_file but with the markers (with hex
strings inside corresponding to the function addresses) resolved to symbols
(names of the functions) in a human readable format with a layout that
emphasizes the function calls nesting.

E.g.:

Call the script:
./backtrace.py --stdout_file qemu_stdout.txt --symbols_file
build-zynq7000-Debug-httpd/os_system/httpServer.instance.bin.lst
```

#### Example Input

With activated function instrumentation, the standard output of the
system includes the entry or exit markers and the function addresses
(e.g. "**`0x25a00 {`**" or "**`0x25a00 }`**") and is used as an
input for the script:

```console
Booting all finished, dropped to user space
main@main.c:2125 Starting CapDL Loader...
main@main.c:2127 CapDL Loader done, suspending...
0x25a00 {
0x25950 {
0x2e35c {
   INFO: /host/trentos_sdk/components/UART/Uart.c:235: initialize UART
   INFO: /host/trentos_sdk/components/UART/Uart.c:295: initialize UART ok
   INFO: /host/httpd/components/Ticker/src/Ticker.c:14: Ticker running
   INFO: /host/httpd/components/NwStack/src/NwStack.c:59: [NwStack 'nwStack'] starting
   INFO: /host/trentos_sdk/components/NIC_ChanMux/driver.c:19: [NIC 'nwDriver'] post_init()
   INFO: /host/trentos_sdk/components/NIC_ChanMux/driver.c:67: [NIC 'nwDriver'] starting driver
   INFO: /host/trentos_sdk/libs/chanmux_nic_driver/src/chanmux_nic_drv_cfg.c:167: network driver init
   INFO: /host/trentos_sdk/libs/chanmux_nic_driver/src/chanmux_nic_drv_cfg.c:182: ChanMUX channels: ctrl=4, data=5
0x2e35c }
0x30fe0 {
0x30fe0 }
0x2e53c {
0x2e53c }
0x25950 }
0x25a00 }
```

#### Example Output

An exemplary output of the **`backtrace.py`** script shows the function
names with their nesting level together with the usual standard output:

```console
Booting all finished, dropped to user space
main@main.c:2125 Starting CapDL Loader...
main@main.c:2127 ESC[0mESC[32mCapDL Loader done, suspending...
_GNUC_init_helper_MHD_init() {
| MHD_init() {
| | MHD_monotonic_sec_counter_init() {
   INFO: /host/trentos_sdk/components/UART/Uart.c:235: initialize UART
   INFO: /host/trentos_sdk/components/UART/Uart.c:295: initialize UART ok
   INFO: /host/httpd/components/Ticker/src/Ticker.c:14: Ticker running
   INFO: /host/httpd/components/NwStack/src/NwStack.c:59: [NwStack 'nwStack'] starting
   INFO: /host/trentos_sdk/components/NIC_ChanMux/driver.c:19: [NIC 'nwDriver'] post_init()
   INFO: /host/trentos_sdk/components/NIC_ChanMux/driver.c:67: [NIC 'nwDriver'] starting driver
   INFO: /host/trentos_sdk/libs/chanmux_nic_driver/src/chanmux_nic_drv_cfg.c:167: network driver init
   INFO: /host/trentos_sdk/libs/chanmux_nic_driver/src/chanmux_nic_drv_cfg.c:182: ChanMUX channels: ctrl=4, data=5
| | MHD_monotonic_sec_counter_init() }
| | MHD_send_init_static_vars_() {
| | MHD_send_init_static_vars_() }
| | MHD_init_mem_pools_() {
| | MHD_init_mem_pools_() }
| MHD_init()
_GNUC_init_helper_MHD_init() }
```
