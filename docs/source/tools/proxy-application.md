# Proxy Application

## General

The proxy application provides a Linux host backend for a TRENTOS
instance running within a QEMU environment. Data exchange is done with
the [ChanMux](../components/chan-mux.md) component running on a virtual serial
port emulated by QEMU, which can either be redirected to a (Linux only) Pseudo
TTY (**PTY**) or any network endpoint via **TCP**. The ChanMux/Proxy protocol
itself is packet based using the **HDLC** frame. This allows multiplexing
different types of a logical communication channel (e.g. Ethernet, NVM
storage) used by TRENTOS via a single physical interface, providing
platform-independent peripheral I/O support.

Currently, the following backend options exist:

- Network Communication: access to Linux **TAP** interfaces
- Persistent Storage: simulation of Non-Volatile Memory (**NVM**)

In coordination with ChanMux, the proxy, therefore, offers several
different channels which represent a specific communication type.
Currently, the following channels are offered (see also
**`enum UartSocketLogicalChannelConvention`**):

- 0-3: reserved for legacy reasons
- 4/5: Network Interface \#1 (Control/Data)
- 6: NVM \#1
- 7/8: Network Interface \#2 (Control/Data)
- 9: NVM \#2
- 10/11: reserved for tests

When the proxy receives new data from QEMU, it extracts the channel IDs
(e.g. 6 for NVM \#1) from the HDLC frames and redirects the payload to
the corresponding backend (e.g. NVM), which will then proceed further
with processing (e.g. write data to a file on the host system
representing the NVM storage). Possible results generated will be sent
back to QEMU as the response.

Currently, the organization and assignment of the channels are static
and must be defined at compile time. This affects several configuration
parameters and especially the size of the NVM file(s) created, which is
defined in **`Nvm.h`**.

```c
#define CHANNEL_NVM_1_MEM_SIZE  (36*1024*1024)  // 36 MiB of memory
#define CHANNEL_NVM_2_MEM_SIZE  (128*1024)      // 128 KiB of memory
```

## Build

The SDK comes with a prebuilt version in **`sdk/bin/proxy_app`**.

But it can also be built from the sources in **`sdk/tools/proxy`** by
running script **`sdk/tools/proxy/build.sh`** in the **trentos_build**
docker container with the absolute path to the SDK as a parameter:

```shell
# run sdk/tools/proxy/build.sh in the container,
# parameter is the absolute path to the SDK (within the container)
sdk/scripts/open_trentos_build_env.sh sdk/tools/proxy/build.sh sdk
```

This will create a separate folder **`build_proxy`**, which contains
the application binary.

## Run

QEMU has to be started first by using the script **`sdk/scripts/run_qemu.sh`**
with the connection type **PTY** or **TCP**:

```shell
sdk/scripts/run_qemu.sh <system-image> [<connectionType>]
```

Then the proxy application is started with the same connection type:

```shell
sdk/bin/proxy_app -c [<connectionType>:<Param>] -t [tap_mode]
```

- connectionType: **PTY** or **TCP**
- Param: **`/dev/pts/<NUMBER>`**  for **PTY** or
    **`<PORT_NUMBER>`** for **TCP**
- TAP mode: \"**`-t 1`**\" indicates that the proxy creates two TAP
    devices that can be used in QEMU for two network cards. This is the
    only mode that is implemented so far.

### Running with PTY

QEMU has to be started accordingly by providing a system image and the
connection type **PTY** as parameters:

```shell
sdk/scripts/run_qemu.sh <SYSTEM_BUILD_DIR>/images/os_image.elf PTY
```

QEMU will then be started, freeze and show an info message which
includes the used PTS number (e.g. 5):

```console
char device redirected to /dev/pts/5 (label serial0)
```

In a second terminal, the proxy has to be started with the PTS device
number (e.g. 5):

```shell
sdk/bin/proxy_app -c PTY:/dev/pts/5 -t 1
```

To unfreeze QEMU and boot, press \<Ctrl-A\> and \<C\> in the QEMU\'s
terminal. Here \<Ctrl-A\> is the shortcut to enter the QEMU monitor console and
\<C\> is the command to continue execution.

### Running with TCP

QEMU has to be started accordingly by providing a system image  and the
connection type **TCP** as parameters:

```shell
sdk/scripts/run_qemu.sh <SYSTEM_BUILD_DIR>/images/os_image.elf TCP
```

QEMU will then be started and waits for an incoming connection from the
proxy application on the port. By default, the scripts use port 4444.

```console
qemu-system-arm: -serial tcp:localhost:4444,server:
info: QEMU waiting for connection on: disconnected:tcp:127.0.0.1:4444,server
```

In a second terminal, the proxy has to be started with the **TCP** port:

```shell
sdk/bin/proxy_app -c TCP:4444 -t 1
```

Now, QEMU unfreezes and boots the system.
