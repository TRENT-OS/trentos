# TlsServer

## Overview

The TlsServer component has the task of encapsulating socket connection
handling (e.g., connect, disconnect) and TLS protocol operations (e.g.,
handshake, etc.) in an isolated component.

**Info:** This component does not implement a "TLS server" in the sense of being
a TLS protocol endpoint with a server socket. "Server" here is to be understood
in terms of RPC communication in line with other TRENTOS components, like
CertServer, StorageServer, etc.

### Concepts

In contrast to the [CryptoServer](crypto-server.md) component, which is
considered as trusted and thus needs to be isolated from the rest of the system,
here the trust relationship is inverted. We intentionally put all the complexity
of the TLS protocol into a separate component because, if this component is
compromised, it cannot easily affect the rest of the system.

### Architecture

The TlsServer makes use of already existing libraries: the
[Socket API](../api/socket_api.md) and the [TLS API](../api/tls_api.md).

In the following diagram, we see that the TlsServer exposes the
**`if_TlsServer`** CAmkES RPC interface. This interface contains the RPC
functions of **`if_OS_Tls`**.

- **`if_OS_Tls`**: This is the RPC interface internally used by the
    TRENTOS TLS API. As a result, the TlsServer can be used as RPC
    server for a TLS API instance configured in
    **`OS_Tls_MODE_CLIENT`** mode.
- **`if_TlsServer:`** This interface adds functions to
    connect/disconnect the client's socket managed by the TlsServer.

The TlsServer allows up to 8 client components to be connected. It uses
for all clients the same certificate to connect to a TLS target host,
that is defined by the clients.

```{mermaid}
flowchart TD
    %% TlsServer Example Package
    subgraph TlsServer_example["TlsServer example"]
        %% Together Clients
        subgraph clients
            Client_1[client1:ClientApp]
            Client_2[client2:ClientApp]
            Client_3[client3:ClientApp]
        end

        TlsServer1[tlsServer:TlsServer]
        NwStack[nwStack]

        %% Connections
        Client_1 -->|"if_TlsServer"| TlsServer1
        Client_2 -->|"if_TlsServer"| TlsServer1
        Client_3 -->|"if_TlsServer"| TlsServer1
        TlsServer1 -->|"if_OS_Socket"| NwStack
    end
```

On the side of the client applications, the RPC functions are not directly
called, but through API functions defined in the [TLS API](../api/tls_api.md) and
the TlsServer client library.

- Functionality related to **`if_OS_Tls`** is accessed through an
    instance of the TLS API library, which internally implements an RPC client.
- Functionality related to **`if_TlsServer`** is accessed through
    **`TlsServer_client`** module, which internally implements an RPC client.

### Implementation

The TlsServer currently supports up to 8 clients and uses a
pre-configured certificate to validate TLS connections. Each client can
open a single TLS connection at a time.

All RPC calls are realized in a non-blocking scheme. The TlsServer does
not offer notifications, so client applications need to implement a
polling behavior.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The TlsServer can be simply declared in the **`CMakeLists.txt`** with
its specialized macro.

```CMake
TlsServer_DeclareCAmkESComponent(
    TlsServer
)
```

For clients of this component, there is a client library that wraps the
raw RPC interface into convenient functions. This interface can be
linked to the client component as follows:

```CMake
DeclareCAmkESComponent(
    <Client>
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_crypto
        os_tls
        TlsServer_client
)
```

Please note that **`os_tls`** and **`os_crypto`** need to be added as well, to
be able to use the TlsServer through the [TLS API](../api/tls_api.md)

### Instantiation and Configuration in CAmkES

To set up the TlsServer in CAmkES the following steps have to be
considered.

#### Adding the interface to the clients

The client component needs to include the **`if_TlsServer`** interface via
the macro **`IF_TLSSERVER_USE`** together with a prefix for the connection.

```c
#include "TlsServer/camkes/if_TlsServer.camkes"

component <ClientComponent> {
    control;
    IF_TLSSERVER_USE(<client_n_if_TlsServer_prefix>)
}
```

#### Declaring the Component

The component can be simply declared as such:

```c
#include "TlsServer/camkes/TlsServer.camkes"
TlsServer_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

Please note that the component's name must match the name given in the
CMake file.

### Instantiating and Connecting the Component

The TlsServer requires an entropy source (for its internal instance of
the [Crypto API](../api/crypto_api.md)) and a connection to the network. For this
use the provided macros of entropy source and network stack to establish the
connections.

In the following the macros of the components **`EntropySource`** and
**`NetworkStack_PicoTcp`** are used. Please refer to their documentation for
further information, e.g. on required configuration macros.

```c
component <NameOfComponent> <nameOfComponent>;

EntropySource_INSTANCE_CONNECT_CLIENT(
    <nameOfEntropySourceInstance>,
    <nameOfInstance>.entropy_rpc , <nameOfInstance>.entropy_port
)

NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS(
    <nameOfNwStackInstance>,
    <nameOfInstance>, networkStack
)
```

To connect clients to the TlsServer use the following macro:

```c
TlsServer_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>
    <client_1_instance>, <client_1_if_TlsServer_prefix>,
    <client_2_instance>, <client_2_if_TlsServer_prefix>,
    ....
)
```

### Configuring the Instance

The TlsServer needs to receive a trusted certificate as part of its
configuration. This certificate will then be used to verify any server
certificates exchanged during the TLS handshake protocol.

```c
TlsServer_INSTANCE_CONFIGURE(
    <nameOfInstance>,
    <CertData>
)
```

### Assigning Clients' Badges

Additionally, the TlsServer requires the clients to have a badge
assigned to their CAmkES interfaces, which can be accomplished through
the corresponding macro:

```c
TlsServer_CLIENT_ASSIGN_BADGES(
    <client_1_instance>, <client_1_if_TlsServer_prefix>,
    <client_2_instance>, <client_2_if_TlsServer_prefix>,
    ....
)
```

## Example

In the following example, we show how to instantiate the TlsServer and
connect two clients to it.

### Instantiation of the Component in CMake

Here we instantiate the TlsServer with the [Socket API](../api/socket_api.md) and
the [Crypto API](../api/crypto_api.md).

```CMake
TlsServer_DeclareCAmkESComponent(
    MyTlsServer
)
```

The following part links the client library of the TlsServer to the
client component:

```CMake
DeclareCAmkESComponent(
    Client_1
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        os_tls
        os_crypto
        TlsServer_client
)

DeclareCAmkESComponent(
    Client_2
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        os_tls
        os_crypto
        TlsServer_client
)
```

### Instantiation and Configuration in CAmkES

See above, just with concrete names and parameters.

#### Adding the interface to the clients

The component can be simply declared:

```c
#include "TlsServer/camkes/if_TlsServer.camkes"

component Client_1 {
    control;
    IF_TLSSERVER_USE(tls)
}

component Client_2 {
    control;
    IF_TLSSERVER_USE(tls)
}
```

#### Declaring the Component

The component can be simply declared:

```c
#include "TlsServer/camkes/TlsServer.camkes"
TlsServer_COMPONENT_DEFINE(
    MyTlsServer
)
```

#### Instantiating and Connecting the Component

The TlsServer requires an entropy source (for its internal instance of
the [Crypto API](../api/crypto_api.md)) and a connection to the network.

The client of the component has to be connected to the respective CAmkES
endpoint implementing **`if_OS_Tls`** and **`if_TlsServer`**.
In this example, the client uses the name **`server_rpc`** for RPC calls (and
**`server_port`** for a shared dataport buffer).

```c
// Instantiate TlsServer
component MyTlsServer               myTlsServer;
// Instantiate EntropySource + network
component EntropySource             entropySource;
component NetworkStack_PicoTcp      nwStack;
// Instantiate two clients
component Client_1                  client_1;
component Client_2                  client_2;

// Connect interfaces PROVIDED by TlsServer
TlsServer_INSTANCE_CONNECT_CLIENTS(
    myTlsServer,
    client_1, tls,
    client_2, tls
)

// Connect to EntropySource
EntropySource_INSTANCE_CONNECT_CLIENT(
    entropySource,
    myTlsServer.entropy_rpc , myTlsServer.entropy_port
)

NetworkStack_PicoTcp_INSTANCE_CONNECT_CLIENTS(
    nwStack,
    myTlsServer, networkStack
)
```

#### Configuring the Instance

The configuration part allows to assign a PEM-encoded certificate as a
trusted certificate to the TlsServer:

```c
TlsServer_INSTANCE_CONFIGURE(
    MyTlsServer,
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDuzCCAqOgAwIBAgIUIlFP3QticKvSug25KJUVB4mqdlswDQYJKoZIhvcNAQEL\r\n" \
    ...
    "b72aCDbgGcHYm4Po+AgYWs4pYP62x7T44xdUYR1QuTb/3J5RMgIcvzngZdD64IFI\r\n" \
    "geBoqyeoBvba6XuFFX7QIX6c39n/Is4aU98GsQHeGY9BCXx9PhNojDKfysyvPGI=\r\n" \
    "-----END CERTIFICATE-----\r\n"
)
```

The following assigns a badge to the client's RPC endpoint:

```c
TlsServer_CLIENT_ASSIGN_BADGES(
    client_1, tls,
    client_2, tls
)
```

### Using the Components Interfaces in C

The code below uses the TlsServer and [TLS API](../api/tls_api.md) (for simplicity,
error checking is not done here):

```c
// Include TLS API
#include "OS_Tls.h"

// Include TlsServer client library
#include "TlsServer_client.h"

// For RPC and dataports
#include <camkes.h>

// Some remote server running TLS 1.2
#define TLS_HOST_IP     "198.166.25.19"
#define TLS_HOST_PORT   443

// TlsServer client interface as defined in CAmkES
static const if_TlsServer_t tlsServer =
    IF_TLSSERVER_ASSIGN(tls);

// Configuration of the TLS API in "CLIENT" mode.
// Use the RPC interface (and dataport) as defined in CAmkES.
static const OS_Tls_Config_t tlsCfg =
{
    .mode = OS_Tls_MODE_CLIENT,
    .rpc = IF_OS_TLS_ASSIGN(tls)
};

int run()
{
    OS_Tls_Handle_t hTls;
    OS_Error_t err;
    const char request[] = "HELLOOOOOO?!";
    #define READ_BUFFER_SIZE 1024
    unsigned char buffer[READ_BUFFER_SIZE];
    size_t len;

    // Tell the TlsServer component to establish a connection to a
    // remote host
    do
    {
        seL4_Yield();
        err = TlsServer_connect(&tlsServer, TLS_HOST_IP, TLS_HOST_PORT);
    }
    while (err == OS_ERROR_WOULD_BLOCK);

    // Initialize the API with the given params
    OS_Tls_init(&hTls, &tlsCfg);

    // Perform handshake through connected socket
    do
    {
        seL4_Yield();
        err = OS_Tls_handshake(hTls);
    }
    while (err == OS_ERROR_WOULD_BLOCK);

    len = sizeof(request);
    // Write something and read the answer
    do
    {
        seL4_Yield();
        err = OS_Tls_write(hTls, request, &len);
    }
    while (err == OS_ERROR_WOULD_BLOCK);

    len = READ_BUFFER_SIZE;
    do
    {
        seL4_Yield();
        err = OS_Tls_read(hTls, buffer, &len);
    }
    while (err == OS_ERROR_WOULD_BLOCK);

    ...

    // Clean up
    OS_Tls_free(hTls);

    // Disconnect the socket; typically this is not needed because
    // the TLS endpoint will disconnect after OS_Tls_free().
    TlsServer_disconnect(&tlsServer);
}
```

We can see here that the same CAmkES endpoint (**`server_rpc`**) is referenced to
assign the **`if_TlsServer`** interface to **`tlsServer`**, as well as to assign
**`if_OS_Tls`** to **`tlsCfg.rpc`**.
