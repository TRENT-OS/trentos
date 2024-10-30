# Socket API

## Overview

The Socket API provides functions to establish and use TCP and UDP communication
channels. The implementation has similarities to BSD Sockets but is not fully
compatible.

The API includes client-side function implementations to communicate with a
Network Stack CAmkES component via the **`if_OS_Socket`** interface.

### Architecture

Within TRENTOS networking functionality is realized the following way:

- NIC driver CAmkES components implement the driver to the ethernet
    hardware. These components provide the **`if_OS_Nic`** interface.
- A Network Stack CAmkES component implements the IP Stack
    functionality.
  - It uses the interface **`if_OS_Nic`** to communicate to a NIC
        driver. There is a 1:1 relationship between those components.
  - It provides the interface **`if_OS_Socket`** to client CAmkES
        components, that require network functionality. The interface is
        designed so that up to 8 clients can connect to one Network
        Stack. The specific capabilities and client numbers depend on
        the Network Stack component implementation.
- A client CAmkES component that requires network functionality
  - uses the interface **`if_OS_Socket`** to communicate with the Network Stack
        component.
  - Instead of directly using the functions of **`if_OS_Socket`**, it uses the
        Socket API to do so.
  - Clients can open several sockets at a time.
  - A client could be connected to several Network Stacks at the same time.

The Socket API is implemented in a non-blocking way. That means that
every API call will return immediately, but some calls will have a
delayed response. An example of this is **`OS_Socket_connect()`**.

To avoid polling strategies together with the non-blocking API,
**`if_OS_Socket`** implements a notification scheme. The Network Stack
will inform each client about new events related to sockets this client
opened. The clients can retrieve a list of pending events for their
sockets from the Network Stack and react to them accordingly.

The following diagram shows a simplified architecture of a networked
application running on an i.MX6 platform.

```{mermaid}
flowchart TD
    subgraph trentos ["TRENTOS networked application on i.MX6"]
        client1[client1:ClientApp]
        client2[client2:ClientApp]

        nwStack[nwStack:NetworkStack_PicoTcp]
        timeserver[timeServer:Timeserver]
        nicDriver[nic:NIC_iMX6]

        client1 -->|if_OS_Socket| nwStack
        client2 -->|if_OS_Socket| nwStack
        nwStack -->|if_OS_Nic| nicDriver
        nwStack -->|if_OS_Timer| timeserver
    end
```

The following diagram shows a simplified diagram of a networked application
running in QEMU.

```{mermaid}
flowchart TD
    subgraph trentos ["TRENTOS networked application on QEMU with ChanMux"]
        subgraph clientApps ["Clients"]
            Client_1[client1:ClientApp]
            Client_2[client2:ClientApp]
        end
        subgraph nwStacks ["Network Stacks"]
            NwStack_1[nwStack1:NetworkStack_PicoTcp]
            NwStack_2[nwStack2:NetworkStack_PicoTcp]
        end
        NIC_Driver_1[nic1:NIC_ChanMux]
        NIC_Driver_2[nic2:NIC_ChanMux]
        Timeserver[timeServer:Timeserver]
        subgraph chanMuxBridge ["ChanMux UART Bridge"]
            ChanMux[chanmux:ChanMux]
        end

        Client_1 -->|if_OS_Socket| NwStack_1
        Client_2 -->|if_OS_Socket| NwStack_2
        NwStack_1 -->|if_OS_Timer| Timeserver
        NwStack_2 -->|if_OS_Timer| Timeserver
        NwStack_1 -->|if_OS_Nic| NIC_Driver_1
        NwStack_2 -->|if_OS_Nic| NIC_Driver_2
        NIC_Driver_1 <-->|Ctrl and Data Channel\nfor TAP Device 1| ChanMux
        NIC_Driver_2 <-->|Ctrl and Data Channel\nfor TAP Device 2| ChanMux
    end

    subgraph linuxHost ["Linux on Host PC"]
        subgraph proxyApp ["Proxy Application"]
            tap_1[TAP Device 1]
            tap_2[TAP Device 2]
            proxy[Proxy]
            proxy <--> tap_1
            proxy <--> tap_2
        end
        LinuxTAP[Linux network subsystem]
        tap_1 <--> LinuxTAP
        tap_2 <--> LinuxTAP
    end

    ChanMux -.-> proxy
```

### Limitations

- Only IPv4 connections are supported

## Usage

This is how the API can be used by a user component.

### Declaration of API Library in CMake

To use the Socket API, a client needs to link the following project
libraries in the CMake file.

- **os_core_api**: Generic build target to include the TRENTOS core API
- **os_socket_client**: Generic build target to include the Socket API client
    implementation

```CMake
DeclareCAmkESComponent(
    UserComponent
    INCLUDES
        ...
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        os_core_api
        os_socket_client
)
```

### Including CAmkES Connectors in the Component Definition

The CAmkES component that wants to use the Socket API needs to include the macro
**`IF_OS_SOCKET_USE`** in its component definition together with a prefix. The
prefix is a unique identifier for that client connection to a specific Network
Stack instance. It needs to be used consistently also in other macros like
**`IF_OS_SOCKET_ASSIGN`** and the macros that connect the client component to
the Network Stack component like
**`NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS`**.

```c
#include <if_OS_Socket.camkes>

component Client {
    IF_OS_SOCKET_USE(<if_OS_Socket_prefix>)
}
```

### Using the API in C

In the implementation, the client needs to set up the RPC lookup table with the
macro **`IF_OS_SOCKET_ASSIGN`** and the \<prefix\> the client used for
**`IF_OS_SOCKET_USE`** .

The created variable (in the example below called **`networkStackCtx`**) can
then be used in the **`OS_Socket_xxx()`** functions provided by the Socket API
as the ctx parameter.

```c
#include "OS_Error.h"

#include "OS_Socket.h"
#include "interfaces/if_OS_Socket.h"
#include <camkes.h>

static const if_OS_Socket_t networkStackCtx =
    IF_OS_SOCKET_ASSIGN(<if_OS_Socket_prefix>);

...

OS_Error_t err;
OS_Socket_Handle_t hsocket;

err = OS_Socket_create(
         &networkStackCtx,
         &hsocket,
         OS_AF_INET,
         OS_SOCK_STREAM);

...

const OS_Socket_Addr_t dstAddr =
{
    .addr = "192.168.1.42",
    .port = 12345
};

err = OS_Socket_connect(hsocket, &dstAddr);

...

OS_Socket_close(hsocket);

...
```

### API Functions Overview

In the following overview, the API functions will be briefly explained.

#### OS_Socket_create

This function will create a new socket handle and connect it to a Network Stack
context.

```c
OS_Error_t
OS_Socket_create(
    const if_OS_Socket_t* const ctx,
    OS_Socket_Handle_t* const   phandle,
    const int                   domain,
    const int                   type);
```

Parameters:

| Parameter | Description                                            | Supported Values                                                    |
|-----------|--------------------------------------------------------|---------------------------------------------------------------------|
| ctx       | Interface context that should be used with the handle. |                                                                     |
| phandle   | Handle that will be assigned to the created socket.    |                                                                     |
| domain    | Domain of the socket that should be created.           | OS_AF_INET                                                          |
| type      | Type of the socket that should be created.             | OS_SOCK_STREAM - for TCP sockets<br>OS_SOCK_DGRAM - for UDP sockets |

Return values:

| Error Code                            | Description                                            |
|---------------------------------------|--------------------------------------------------------|
| OS_SUCCESS                            | Operation was successful.                             |
| OS_ERROR_ABORTED                      | If the Network Stack has experienced a fatal error.   |
| OS_ERROR_NOT_INITIALIZED              | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_PARAMETER            | If an invalid parameter or NULL pointer was passed.   |
| OS_ERROR_NETWORK_PROTO_NO_SUPPORT     | If the passed domain or type is unsupported.          |
| OS_ERROR_NETWORK_UNREACHABLE          | If the network is unreachable.                         |
| OS_ERROR_INSUFFICIENT_SPACE           | If no free sockets could be found.                     |
| other                                 | Each component implementing this might have additional error codes. |

Particularities:

- None

#### OS_Socket_connect

This function will initiate a connection to a remote server.

```c
OS_Error_t
OS_Socket_connect(
    const OS_Socket_Handle_t handle,
    const OS_Socket_Addr_t*  dstAddr);
```

Parameters:

| Parameter |                Description                |          Supported Values          |
|-----------|:-----------------------------------------:|:----------------------------------:|
| handle    | Handle of the socket to connect.          |                                    |
| dstAddr   | Address of the destination to connect to. | Only IPv4 Addresses are supported. |

Return values:

| Error Code                        |                                 Description                                |
|-----------------------------------|:--------------------------------------------------------------------------:|
| OS_SUCCESS                        | Operation was successful.                                                  |
| OS_ERROR_ABORTED                  | If the Network Stack has experienced a fatal error.                        |
| OS_ERROR_NOT_INITIALIZED          | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_HANDLE           | If an invalid handle was passed.                                           |
| OS_ERROR_INVALID_PARAMETER        | If an invalid parameter or NULL pointer was passed.                        |
| OS_ERROR_CONNECTION_CLOSED        | If the socket connection was previously shut down.                         |
| OS_ERROR_NETWORK_PROTO_NO_SUPPORT | If the protocol is not supported.                                          |
| OS_ERROR_NETWORK_HOST_UNREACHABLE | If the host is not unreachable.                                            |
| OS_ERROR_NETWORK_PROTO            | If the function is called on the wrong socket type.                        |
| other                             | Each component implementing this might have additional error codes.        |

Particularities:

- A return value of **`OS_SUCCESS`** does not mean, that the
    connection is established, but that the connection process was
    successfully initiated.
- The Network Stack will inform the client with a notification about
    the connection process result.
  - A successful connection process will be reported with
        **`OS_SOCK_EV_CONN_EST`** .
  - An unsuccessful connection attempt will be reported with
        **`OS_SOCK_EV_FIN`** or **`OS_SOCK_EV_ERROR`** .
- If during the connection process **`OS_Socket_read()`** or
    **`OS_Socket_write()`** are called, the functions will return with
    **`OS_ERROR_NETWORK_CONN_NONE`** .
- If after an unsuccessful connection process **`OS_Socket_read()`**
    or **`OS_Socket_write()`** are called, the functions will return
    with **`OS_ERROR_CONNECTION_CLOSED`** .
- Following an unsuccessful connection process, the socket handle is
    not usable anymore and needs to be closed with
    **`OS_Socket_close()`** .
- If during the connection process **`OS_Socket_close()`** is called,
    the connection attempt will be canceled.

#### OS_Socket_bind

This function will bind a specified local IP address and port to a
socket.

```c
OS_Error_t
OS_Socket_bind(
    const OS_Socket_Handle_t      handle,
    const OS_Socket_Addr_t* const localAddr);
```

Parameters:

| Parameter | Description                          | Supported Values                   |
|-----------|--------------------------------------|------------------------------------|
| handle    | Handle of the socket to bind.        |                                    |
| localAddr | Local address to bind the socket to. | Only IPv4 Addresses are supported. |

Return values:

| Error Code                  | Description                                                                |
|-----------------------------|----------------------------------------------------------------------------|
| OS_SUCCESS                  | Operation was successful.                                                  |
| OS_ERROR_ABORTED            | If the Network Stack has experienced a fatal error.                        |
| OS_ERROR_NOT_INITIALIZED    | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_HANDLE     | If an invalid handle was passed.                                           |
| OS_ERROR_INVALID_PARAMETER  | If an invalid parameter or NULL pointer was passed.                        |
| OS_ERROR_IO                 | If the specified address can not be found.                                 |
| OS_ERROR_INSUFFICIENT_SPACE | If there is not enough space.                                              |
| OS_ERROR_CONNECTION_CLOSED  | If the connection is in a closed state.                                    |
| other                       | Each component implementing this might have additional error codes.        |

Particularities:

- None

#### OS_Socket_listen

This function will listen for incoming connections on an opened and
bound socket.

```c
OS_Error_t
OS_Socket_listen(
    const OS_Socket_Handle_t handle,
    const int                backlog);
```

Parameters:

| Parameter | Description                                                               | Supported Values |
|-----------|---------------------------------------------------------------------------|------------------|
| handle    | Handle of the socket to listen on.                                        |                  |
| backlog   | Sets the maximum size to which the queue of pending connections may grow. |                  |

Return values:

| Error Code                          | Description                                                                |
|-------------------------------------|----------------------------------------------------------------------------|
| OS_SUCCESS                          | Operation was successful.                                                  |
| OS_ERROR_ABORTED                    | If the Network Stack has experienced a fatal error.                        |
| OS_ERROR_NOT_INITIALIZED            | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_HANDLE             | If an invalid handle was passed.                                           |
| OS_ERROR_INVALID_PARAMETER          | If an invalid parameter or NULL pointer was passed.                        |
| OS_ERROR_CONNECTION_CLOSED          | If the connection is in a closed state.                                    |
| OS_ERROR_NETWORK_CONN_ALREADY_BOUND | If the socket is already connected.                                        |
| OS_ERROR_NETWORK_PROTO              | If the function is called on the wrong socket type.                        |
| other                               | Each component implementing this might have additional error codes.        |

Particularities:

- None

#### OS_Socket_accept

Accept the next connection request on the queue of pending connections
for the listening socket.

```c
OS_Error_t
OS_Socket_accept(
    const OS_Socket_Handle_t  handle,
    OS_Socket_Handle_t* const pClientHandle,
    OS_Socket_Addr_t* const   srcAddr);
```

Parameters:

| Parameter     | Description                                                 | Supported Values |
|---------------|-------------------------------------------------------------|------------------|
| handle        | Handle of the listening socket.                             |                  |
| pClientHandle | Handle that will be used to map the accepted connection to. |                  |
| srcAddr       | Address of the accepted socket.                             |                  |

Return values:

| Error Code                  | Description                                                                     |
|-----------------------------|---------------------------------------------------------------------------------|
| OS_SUCCESS                  | Operation was successful.                                                       |
| OS_ERROR_ABORTED            | If the Network Stack has experienced a fatal error.                             |
| OS_ERROR_NOT_INITIALIZED    | If the function was called before the Network Stack was fully initialized.      |
| OS_ERROR_INVALID_HANDLE     | If an invalid handle was passed.                                                |
| OS_ERROR_INVALID_PARAMETER  | If an invalid parameter or NULL pointer was passed.                             |
| OS_ERROR_TRY_AGAIN          | If the resource is temporarily unavailable and the caller should try again.     |
| OS_ERROR_NETWORK_PROTO      | If the function is called on the wrong socket type.                             |
| OS_ERROR_INSUFFICIENT_SPACE | If no free sockets could be found.                                              |
| OS_ERROR_CONNECTION_CLOSED  | If the connection was closed by remote before it was accepted by the localhost. |
| other                       | Each component implementing this might have additional error codes.             |

Particularities:

- None

#### OS_Socket_read

This function will read data from a socket. This function checks whether
or not the socket is bound and connected before it attempts to receive
data.

```c
OS_Error_t
OS_Socket_read(
    const OS_Socket_Handle_t handle,
    void* const              buf,
    size_t                   requestedLen,
    size_t* const            actualLen);
```

Parameters:

| Parameter    | Description                                  | Supported Values |
|--------------|----------------------------------------------|------------------|
| handle       | Handle of the socket to read from.           |                  |
| buf          | Buffer to store the read data.               |                  |
| requestedLen | Length of the data that should be read.      |                  |
| actualLen    | Actual length that was read from the socket. |                  |

Return values:

| Error Code                     | Description                                                                                            |
|--------------------------------|--------------------------------------------------------------------------------------------------------|
| OS_SUCCESS                     | Operation was successful.                                                                              |
| OS_ERROR_ABORTED               | If the Network Stack has experienced a fatal error.                                                    |
| OS_ERROR_NOT_INITIALIZED       | If the function was called before the Network Stack was fully initialized.                             |
| OS_ERROR_INVALID_HANDLE        | If an invalid handle was passed.                                                                       |
| OS_ERROR_INVALID_PARAMETER     | If an invalid parameter or NULL pointer was passed.                                                    |
| OS_ERROR_IO                    | If there is an input/output error.                                                                     |
| OS_ERROR_NETWORK_CONN_NONE     | If no connection is established when calling this function.                                            |
| OS_ERROR_NETWORK_PROTO         | If the function is called on the wrong socket type.                                                    |
| OS_ERROR_TRY_AGAIN             | If the resource is temporarily unavailable or the request would block and the caller should try again. |
| OS_ERROR_CONNECTION_CLOSED     | If the connection is in a closed state.                                                                |
| OS_ERROR_NETWORK_CONN_SHUTDOWN | If the connection got shut down.                                                                       |
| other                          | Each component implementing this might have additional error codes.                                    |

Particularities:

- **`OS_Socket_read()`** will return as many bytes as have been received from
    the remote host, up to **`requestedLen`**. This may also include data
    received over several ethernet frames.

#### OS_Socket_write

This function will write data on a socket. This function checks if the
socket is bound, connected and that it isn't shut down locally.

```c
OS_Error_t
OS_Socket_write(
    const OS_Socket_Handle_t handle,
    const void* const        buf,
    const size_t             requestedLen,
    size_t* const            actualLen);
```

Parameters:

| Parameter    | Description                                   | Supported Values |
|--------------|-----------------------------------------------|------------------|
| handle       | Handle of the socket to write on.             |                  |
| buf          | Buffer containing data that should be sent.   |                  |
| requestedLen | Amount of data that should be written.        |                  |
| actualLen    | Actual length that was written on the socket. |                  |

Return values:

| Error Code                          | Description                                                                                            |
|-------------------------------------|--------------------------------------------------------------------------------------------------------|
| OS_SUCCESS                          | Operation was successful.                                                                              |
| OS_ERROR_ABORTED                    | If the Network Stack has experienced a fatal error.                                                    |
| OS_ERROR_NOT_INITIALIZED            | If the function was called before the Network Stack was fully initialized.                             |
| OS_ERROR_INVALID_HANDLE             | If an invalid handle was passed.                                                                       |
| OS_ERROR_INVALID_PARAMETER          | If an invalid parameter or NULL pointer was passed.                                                    |
| OS_ERROR_IO                         | If there is an input/output error.                                                                     |
| OS_ERROR_NETWORK_PROTO              | If the function is called on the wrong socket type.                                                    |
| OS_ERROR_INSUFFICIENT_SPACE         | If there is not enough space.                                                                          |
| OS_ERROR_TRY_AGAIN                  | If the resource is temporarily unavailable or the request would block and the caller should try again. |
| OS_ERROR_CONNECTION_CLOSED          | If the connection is in a closed state.                                                                |
| OS_ERROR_NETWORK_CONN_NONE          | If the socket is not connected.                                                                        |
| OS_ERROR_NETWORK_CONN_SHUTDOWN      | If the connection got shut down.                                                                       |
| OS_ERROR_NETWORK_ADDR_NOT_AVAILABLE | If the address is not available.                                                                       |
| OS_ERROR_NETWORK_HOST_UNREACHABLE   | If the host is not unreachable.                                                                        |
| other                               | Each component implementing this might have additional error codes.                                    |

Particularities:

- With **`OS_Socket_write()`** the network stack will copy data to be sent into
    it's internal buffers and schedule it for actual transmission later. If
    errors occur during the actual transmission, those will be reported by
    **`OS_Socket_getPendingEvents()`** / **`OS_SOCK_EV_ERROR`**.
- Depending on the internal buffer capacity of the network stack,
    **`OS_Socket_write()`** may accept only partially the data
    in buf. It will return the number of successfully accepted bytes in
    **`actualLen`**.

#### OS_Socket_recvfrom

This function will receive data from a specified socket. This operation
checks if the socket is bound but not if it is connected and is
therefore not connection-oriented.

```c
OS_Error_t
OS_Socket_recvfrom(
    const OS_Socket_Handle_t handle,
    void* const              buf,
    size_t                   requestedLen,
    size_t* const            actualLen,
    OS_Socket_Addr_t* const  srcAddr);
```

Parameters:

| Parameter    | Description                                               | Supported Values |
|--------------|-----------------------------------------------------------|------------------|
| handle       | Handle to a previously created socket.                    |                  |
| buf          | Buffer to store the read data.                            |                  |
| requestedLen | Length of the data that should be read.                   |                  |
| actualLen    | Actual length that was read from the socket.              |                  |
| srcAddr      | Address of the source socket that data was received from. |                  |

Return values:

| Error Code                          | Description                                                                                            |
|-------------------------------------|--------------------------------------------------------------------------------------------------------|
| OS_SUCCESS                          | Operation was successful.                                                                              |
| OS_ERROR_ABORTED                    | If the Network Stack has experienced a fatal error.                                                    |
| OS_ERROR_NOT_INITIALIZED            | If the function was called before the Network Stack was fully initialized.                             |
| OS_ERROR_INVALID_HANDLE             | If an invalid handle was passed.                                                                       |
| OS_ERROR_INVALID_PARAMETER          | If an invalid parameter or NULL pointer was passed.                                                    |
| OS_ERROR_NETWORK_PROTO              | If the function is called on the wrong socket type.                                                    |
| OS_ERROR_TRY_AGAIN                  | If the resource is temporarily unavailable or the request would block and the caller should try again. |
| OS_ERROR_CONNECTION_CLOSED          | If no further communication is possible on the socket.                                                 |
| OS_ERROR_NETWORK_CONN_SHUTDOWN      | If the connection got shut down.                                                                       |
| OS_ERROR_NETWORK_ADDR_NOT_AVAILABLE | If the address is not available.                                                                       |
| other                               | Each component implementing this might have additional error codes.                                    |

Particularities:

- **`OS_Socket_recvfrom()`** will return the contents of one
    received UDP datagram.
- If the datagram had a payload size of 0, **`OS_Socket_recvfrom()`** will
    return with **`OS_SUCCESS`** and **`actualLen`** of 0.
- If requestedLen is smaller then the the payload size of the datagram,
    **`OS_Socket_recvfrom()`** will return the partial data as requested.
    Following calls to **`OS_Socket_recvfrom()`** will return the remaining data
    of the partially read datagram.

#### OS_Socket_sendto

This function will send data on a destination socket without checking if
the destination is connected or not and is therefore not
connection-oriented.

```c
OS_Error_t
OS_Socket_sendto(
    const OS_Socket_Handle_t      handle,
    const void* const             buf,
    size_t                        requestedLen,
    size_t* const                 actualLen,
    const OS_Socket_Addr_t* const dstAddr);
```

Parameters:

| Parameter    | Description                                                          | Supported Values                   |
|--------------|----------------------------------------------------------------------|------------------------------------|
| handle       | Handle to a previously created socket.                               |                                    |
| buf          | Buffer containing data to be written.                                |                                    |
| requestedLen | Length of the data that should be written on the destination socket. |                                    |
| actualLen    | Actual length that was written on the socket.                        |                                    |
| dstAddr      | Address of the destination socket to send data on.                   | Only IPv4 Addresses are supported. |

Return values:

| Error Code                          | Description                                                                                            |
|-------------------------------------|--------------------------------------------------------------------------------------------------------|
| OS_SUCCESS                          | Operation was successful.                                                                              |
| OS_ERROR_ABORTED                    | If the Network Stack has experienced a fatal error.                                                    |
| OS_ERROR_NOT_INITIALIZED            | If the function was called before the Network Stack was fully initialized.                             |
| OS_ERROR_INVALID_HANDLE             | If an invalid handle was passed.                                                                       |
| OS_ERROR_INVALID_PARAMETER          | If an invalid parameter or NULL pointer was passed.                                                    |
| OS_ERROR_INSUFFICIENT_SPACE         | If there is not enough space.                                                                          |
| OS_ERROR_NETWORK_PROTO              | If the function is called on the wrong socket type.                                                    |
| OS_ERROR_TRY_AGAIN                  | If the resource is temporarily unavailable or the request would block and the caller should try again. |
| OS_ERROR_CONNECTION_CLOSED          | If no further communication is possible on the socket.                                                 |
| OS_ERROR_NETWORK_ADDR_NOT_AVAILABLE | If the address is not available.                                                                       |
| OS_ERROR_NETWORK_HOST_UNREACHABLE   | If the host is not unreachable.                                                                        |
| other                               | Each component implementing this might have additional error codes.                                    |

Particularities:

- With **`OS_Socket_sendto()`** the network stack will copy data to be sent into
    it's internal buffers and schedule it for actual transmission later. If
    errors occur during the actual transmission, those will be reported by
    **`OS_Socket_getPendingEvents()`** / **`OS_SOCK_EV_ERROR`**.
- Depending on the internal buffer capacity of the network stack,
    **`OS_Socket_sendto()`** may accept only partially the data
    in buf. It will return the number of successfully accepted bytes in
    **`actualLen`**.

#### OS_Socket_close

This function will close a network socket. No further socket
communication is possible after closure.

```c
OS_Error_t
OS_Socket_close(
    const OS_Socket_Handle_t handle);
```

Parameters:

| Parameter | Description                                 | Supported Values |
|-----------|---------------------------------------------|------------------|
| handle    | Handle of the socket that should be closed. |                  |

Return values:

| Error Code                 | Description                                                                |
|----------------------------|----------------------------------------------------------------------------|
| OS_SUCCESS                 | Operation was successful.                                                  |
| OS_ERROR_ABORTED           | If the Network Stack has experienced a fatal error.                        |
| OS_ERROR_NOT_INITIALIZED   | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_HANDLE    | If an invalid handle was passed.                                           |
| OS_ERROR_INVALID_PARAMETER | If the handle context is invalid.                                          |
| other                      | Each component implementing this might have additional error codes.        |

Particularities:

- None

#### OS_Socket_getPendingEvents

This function will get the pending events for the opened sockets.

```c
OS_Error_t
OS_Socket_getPendingEvents(
    const if_OS_Socket_t* const ctx,
    void* const                 buf,
    const size_t                bufSize,
    int* const                  numberOfEvents);
```

Parameters:

| Parameter      | Description                                    | Supported Values |
|----------------|------------------------------------------------|------------------|
| ctx            | Interface context that should be used.         |                  |
| buf            | Buffer to store the event data.                |                  |
| bufSize        | Size of the buffer to store the event data.    |                  |
| numberOfEvents | Will be overwritten with the number of events. |                  |

Return values:

| Error Code                 | Description                                                                |
|----------------------------|----------------------------------------------------------------------------|
| OS_SUCCESS                 | Operation was successful.                                                  |
| OS_ERROR_ABORTED           | If the Network Stack has experienced a fatal error.                        |
| OS_ERROR_NOT_INITIALIZED   | If the function was called before the Network Stack was fully initialized. |
| OS_ERROR_INVALID_HANDLE    | If an invalid handle was passed.                                           |
| OS_ERROR_INVALID_PARAMETER | If an invalid parameter or NULL pointer was passed.                        |
| OS_ERROR_BUFFER_TOO_SMALL  | If the buffer is not even large enough for one event.                      |
| other                      | Each component implementing this might have additional error codes.        |

The events that this function will place in the passed buffer are all of
type **`OS_Socket_Evt_t`**.

```c
typedef struct __attribute__((packed))
{
    int        socketHandle; //!< Handle ID of the socket.
    int        parentSocketHandle; //!< Handle ID of the parent socket.
    uint8_t    eventMask; //!< Event mask of the socket.
    OS_Error_t currentError; //!< Current error of the socket.
}
OS_Socket_Evt_t;
```

The event flags that can be set for the **`eventMask`** member of this
struct and that can be evaluated by the user are listed below.

```c
#define OS_SOCK_EV_NONE      (0)
#define OS_SOCK_EV_CONN_EST  (1<<0) //!< Connection established (TCP only).
#define OS_SOCK_EV_CONN_ACPT (1<<1) //!< Connection accepted (TCP only).
#define OS_SOCK_EV_READ      (1<<2) //!< Data arrived on the socket.
#define OS_SOCK_EV_WRITE     (1<<3) //!< Ready to write to the socket (TCP only).
#define OS_SOCK_EV_FIN       (1<<4) //!< FIN segment received (TCP only).
#define OS_SOCK_EV_CLOSE     (1<<5) //!< Socket is closed (TCP only). No further communication is possible from this point on the socket.
#define OS_SOCK_EV_ERROR     (1<<6) //!< An error occurred.
```

Particularities:

- None

#### OS_Socket_getStatus

This function queries the current state of the Network Stack component.

```c
OS_NetworkStack_State_t
OS_Socket_getStatus(
    const if_OS_Socket_t* const ctx);
```

Parameters:

| Parameter | Description                            | Supported Values |
|-----------|----------------------------------------|------------------|
| ctx       | Interface context that should be used. |                  |

Return values:

| Network Stack State | Description                                  |
|---------------------|----------------------------------------------|
| UNINITIALIZED       | Network Stack is uninitialized.              |
| INITIALIZED         | Network Stack is initialized.                |
| RUNNING             | Network Stack is running.                    |
| FATAL_ERROR         | Network Stack has experienced a fatal error. |

Particularities:

- None

#### OS_Socket_poll

This function checks, if a notification was received from the Network Stack
before.

```c
OS_Error_t
OS_Socket_poll(
    const if_OS_Socket_t* const ctx);
```

Parameters:

| Parameter | Description                                            | Supported Values |
|-----------|--------------------------------------------------------|------------------|
| ctx       | Interface context that should be used with the handle. |                  |

Return values:

| Error Code                 | Description                                                         |
|----------------------------|---------------------------------------------------------------------|
| OS_SUCCESS                 | Notification event found.                                           |
| OS_ERROR_INVALID_PARAMETER | If the handle context is invalid.                                   |
| OS_ERROR_TRY_AGAIN         | If no notification event was found.                                 |
| other                      | Each component implementing this might have additional error codes. |

Particularities:

- None

#### OS_Socket_wait

This function blocks until a notification is received from the Network Stack. If
a notification was received before, this function will return immediately.

```c
OS_Error_t
OS_Socket_wait(
    const if_OS_Socket_t* const ctx);
```

Parameters:

| Parameter | Description                                            | Supported Values |
|-----------|--------------------------------------------------------|------------------|
| ctx       | Interface context that should be used with the handle. |                  |

Return values:

| Error Code                 | Description                                                         |
|----------------------------|---------------------------------------------------------------------|
| OS_SUCCESS                 | Notification event found.                                           |
| OS_ERROR_INVALID_PARAMETER | If the handle context is invalid.                                   |
| other                      | Each component implementing this might have additional error codes. |

Particularities:

- None

#### OS_Socket_regCallback

This function will register a callback function, that is executed once the
Network Stack sends a notification.

```c
OS_Error_t
OS_Socket_regCallback(
    const if_OS_Socket_t* const ctx,
    (void (*callback)(void*),
    void *arg)
);
```

Parameters:

| Parameter | Description                                  | Supported Values  |
|-----------|----------------------------------------------|-------------------|
| ctx       | Interface context that should be used.       |                   |
| callback  | A function pointer to the callback function. |                   |
| arg       | The arguments for the callback functions.    | Can also be NULL. |

Return values:

| Error Code                 | Description                                                         |
|----------------------------|---------------------------------------------------------------------|
| OS_SUCCESS                 | Callback function successfully registered.                          |
| OS_ERROR_INVALID_PARAMETER | If an invalid parameter or NULL pointer was passed.                 |
| OS_ERROR_GENERIC           | If the callback could not be registered.                            |
| other                      | Each component implementing this might have additional error codes. |

Particularities:

- Upon execution of the callback function, the callback is automatically
    unregistered.
- If the callback function is to be executed again for the next notification,
    the callback function needs to re-register itself within its function body.

### Non-Blocking Strategies

#### Socket Handling by Polling

In case you have a simple application with a single client connection, you can
omit using the notification sent out by the Network Stack and just use the
Socket API in polling mode.  All functions are non-blocking, so a user needs to
read the return values and react properly.

For example, **`OS_Socket_connect()`** will return with **`OS_SUCCESS`** in case
the connection process could be initiated successfully. Read and write calls
will be returned with **`OS_ERROR_NETWORK_CONN_NONE`** until the connection is
established. The code example below shows how this could be realized in a
simplified way.

```c
...

err = OS_Socket_create(
    &networkStackCtx,
    &socket,
    OS_AF_INET,
    OS_SOCK_STREAM);

...


err = OS_Socket_connect(socket, &dstAddr);

...


size_t offs = 0;

do
{
    const size_t lenRemaining = len_request - offs;
    size_t       len_io       = lenRemaining;

    err = OS_Socket_write(
              socket,
              &request[offs],
              len_io,
              &len_io);
    if (err != OS_SUCCESS)
    {
        if (err == OS_ERROR_TRY_AGAIN || err == OS_ERROR_NETWORK_CONN_NONE)
        {
   // Try again.
   continue;
        }
        else
        {
            Debug_LOG_ERROR("OS_Socket_write() failed, code %d", err);
            OS_Socket_close(socket);
            return;
        }
    }

    offs += len_io;
}
while (offs < len_request);

...
```

#### Socket Handling by Main Event Loop

For more complex implementations it is advised to use the notification sent out
by the Network Stack and retrieve the details about all pending events via
**`OS_Socket_getPendingEvents()`**.

The NetworkStack_PicoTcp will cyclically process all incoming data and existing
connections. When an update happens for any of the open sockets, it will send a
notification to the client that the socket belongs to. Please note, that no
individual notifications are sent for each socket. Instead, only one
notification is sent after each processing cycle of the Network Stack.

A client can retrieve a list of all events from the Network Stack by calling
**`OS_Socket_getPendingEvents()`**. This will return an array of events for all
updated sockets. The client can then process each event and react appropriately.

One strategy to make use of this is to include an endless loop in the run()
function and wait for the notification to be triggered. After receiving the
notification we retrieve the pending events, act on all events, tidy up tasks,
and wait again on the notification.

```c
...

int
run()
{
    ...

    for (;;)
    {
        // Wait until we get an event for the listening socket.
        OS_Socket_wait(&networkStackCtx);

        err = OS_Socket_getPendingEvents(
                &network_stack,
                evtBuffer,
                evtBufferSize,
                &numberOfSocketsWithEvents);

        ...

        // Verify that the received number of sockets with events is within expected
        // bounds.

     ...

  // Iterate through the received socket events and handle them according to the application logic.
        for (int i = 0; i < numberOfSocketsWithEvents; i++)
        {
   OS_Socket_Evt_t event;
            memcpy(&event, &evtBuffer[offset], sizeof(event));
            offset += sizeof(event);

   // Socket has been closed by the network stack - close socket.
            if (event.eventMask & OS_SOCK_EV_FIN)
            {
                OS_Socket_close(socket);
       continue;
            }

          ...

  }
    }

    ...

}

...
```

#### Socket Handling by Notification Callback

The CAmkES infrastructure for the **`if_OS_Socket`** interface provides the
functionality to register a callback function that is executed once the Network
Stack sends a notification (see also the API documentation above for more
details on how to register a callback function). It is therefore possible to
also implement a scheme to handle the socket events that rely on this mechanism
to update the current event state for all sockets a client is managing.

While this strategy can provide certain benefits it should also be noted that
this typically results in a more complex solution, since the callback function
is executed in a separate thread and can result in concurrency issues.
Additional synchronization primitives will therefore be necessary on the user
application side.

The example below provides a sketch of how this strategy can be implemented and
what API functions would need to be involved for this approach.

```c
...

static OS_Socket_Evt_t eventCollection[OS_NETWORK_MAXIMUM_SOCKET_NO] = {0};

...

static void
cb_nwStackNotification(
    void* ctx)
{

    ...

    OS_Error_t err = OS_Socket_getPendingEvents(
                         ctx,
                         eventBuffer,
                         bufferSize,
                         &numberOfSocketsWithEvents);

 ...

    for (int i = 0; i < numberOfSocketsWithEvents; i++)
    {
        // Update local eventCollection[OS_NETWORK_MAXIMUM_SOCKET_NO] array
        // with the latest events for the sockets. The update of this shared
  // ressource should be mutex protected in case the run() thread is
        // currently also trying to access it.
    }

    // Unblock any caller of the functions below waiting for new events to
    // arrive.
    if (numberOfSocketsWithEvents)
    {
        notify_about_new_events();
    }

    // Re-register the callback function as it should be executed again for the
    // next notification.
    err = OS_Socket_regCallback(
              ctx,
              &cb_nwStackNotification,
              ctx);
 ...

}

...

OS_Error_t
waitForConnectionEstOnSocket(
    const OS_Socket_Handle_t handle)
{
    ...

    for(;;)
    {
        // Lock mutex for shared eventCollection ressource

        eventMask = eventCollection[handle.handleID].eventMask;
        err = eventCollection[handle.handleID].currentError;

        // Unlock mutex for shared eventCollection ressource

  // Check eventMask for the specific events that might be relevant
        // to evaluate.

  // If a relevant event is found to be set in the eventMask
        // (e.g. OS_SOCK_EV_CONN_EST) return OS_SUCCESS. If and error
        // is found with OS_SOCK_EV_ERROR, return err.

  // Else if no relevant event is found, wait on the arrival of new events.
        // The signal would be emitted by the callback function in this example.
    }
}

...

int
run()
{

    ...

    // Set up callback for new received socket events.
    err = OS_Socket_regCallback(
              &networkStackCtx,
              &cb_nwStackNotification,
              (void*) &networkStackCtx);
    ...

 err = OS_Socket_create(
    &networkStackCtx,
    &socket,
    OS_AF_INET,
    OS_SOCK_STREAM);

    ...

    err = OS_Socket_connect(socket, &dstAddr);

    ...

 // Wait until the socket is successfully connected.
    err = waitForConnectionEstOnSocket(handle);

    ...

}

...
```

## Further Examples

To get a better understanding of how the Network Stack can be utilized in a
system, it is recommended to take a look at the demos provided by the SDK.

In the IoT Demos, a Cloud Connector component is connected to a Network Stack
component as a TCP client and uses the networking functionalities to publish
MQTT messages from a Sensor component connected to it.

In the demo_tls_api a TLS connection is established, in which the main component
needs to setup the TCP connection to a remote server.
