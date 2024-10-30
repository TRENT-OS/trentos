# CertServer

## Overview

The CertServer is a component that is configured with a set of trusted
certificates (the "trusted chain") and offers "read access" to these
certs to its clients via a simple interface. The idea here is to isolate
those trusted certificates in a small component so they cannot be
changed.

A CertServer instance can have only one trusted chain, but in a complex
system there can be different CertServers with different chains (e.g.,
one CertServer for TLS-based connections, one CertServer for Secure
Update functionality, etc.).

### Architecture

The CertServer makes use of the [CertParser API](../api/cert-parser_api); for
this reason, the functionality exposed via RPC closely resembles the
functionality known from the CertParser library.

In the following diagram, we see that the CertServer exposes the
**`if_CertServer`** interfaces via the **`certServer_rpc`** RPC endpoint:

```{mermaid}
flowchart TD
    %% Client1 Node
    subgraph Client1
        App1[Application]
        CertServer_client1[CertServer_client]
        App1 --> CertServer_client1
    end

    %% Client2 Node
    subgraph Client2
        App2[Application]
        CertServer_client2[CertServer_client]
        App2 --> CertServer_client2
    end

    %% CertServer Node
    subgraph CertServer
        certServer_rpc[certServer_rpc]
        subgraph CertParser_Library["CertParser Library"]
            OS_CertParser_Server[OS_CertParser]
        end
        CertServer_client1 -. "if_CertServer" .- certServer_rpc
        CertServer_client2 -. "if_CertServer" .- certServer_rpc
        certServer_rpc --> OS_CertParser_Server
    end
```

Please note that on the side of the application component, the
**`certServer_rpc`** endpoint is never accessed directly but always through its
client library.

### Implementation

The CertServer uses the [CertParser API](../api/cert-parser_api) for all
certificate-parsing related operations; this in turn uses mbedTLS for
certificate-parsing and the [Crypto API](../api/crypto_api) for cryptography.

The CertServer requires badges/IDs to be in the interval (101-108) and
supports up to 8 clients. Every client has its own certificate chain,
which it can initialize and add certificates to.

All clients share a single CertParser instance, which is configured with
a trusted certificate via CAmkES attribute. This single instance is used
to verify the client\'s respective chains.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The CertServer can be instantiated via the following:

```CMake
CertServer_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

In order to use the CertServer's client interface, the respective component's
client code has to include it as part of its LIBS parameter:

```CMake
DeclareCAmkESComponent(
    <Client>
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        <NameOfComponent>_client
)
```

### Instantiation and Configuration in CAmkES

The CertServer uses the **`if_OS_Entropy`** interface and offers its own
interface, which is shown here.

#### Declaring the Component

The declaration of the CertServer works as follows:

```c
#include "CertServer/camkes/CertServer.camkes"
CertServer_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

#### Instantiating and Connecting the Component

The CertServer uses **`if_OS_Entropy`** which needs to be set up before
connecting any clients:

```c
component <NameOfComponent> <nameOfInstance>;

CertServer_INSTANCE_CONNECT(
    <nameOfInstance>,
    <entropy>.<nameOfInterface>, <entropy>.<nameOfDataport>
)
```

For its clients, the CertServer offers the **`if_CertServer`** interfaces, which
can be connected to the respective clients as follows:

```c
CertServer_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>,
    <client1>.<nameOfInterface>, <client1>.<nameOfDataport>,
    <client2>.<nameOfInterface>, <client2>.<nameOfDataport>,
    ...
)
```

This list can contain up to 8 clients.

#### Configuring the Instance

The CertServer is configured to perform all certificate verification against a
defined, trusted certificate chain. For this purpose, another macro is used
which has the following parameters:

- **`num:`** Number of PEM encoded certificates to use as trusted chain
- **`cert1, cert2, ...:`** A list of PEM-encoded certificates
    (declared as strings) which form the trusted chain. Please note that
    each certificate needs to be signed by the one preceding it in the
    chain, e.g., **`cert2`** is signed by **`cert1`**, etc. Obviously,
    the number of arguments here must match the **`num`** attribute.

The full macro looks as follows:

```c
CertServer_INSTANCE_CONFIGURE(
    <nameOfInstance>,
    <num>,
    <cert1>,
    <cert2>,
    ...
)
```

The CertServer offers one RPC endpoint, for which the CAmkES badge ID
needs to be assigned explicitly. For this, the following code can be
used:

```c
CertServer_CLIENT_ASSIGN_BADGES(
    <client1>.<nameOfInterface>,
    <client2>.<nameOfInterface>,
    ...
)
```

The order of assignment must be the same for
**`CertServer_INSTANCE_CONNECT_CLIENTS()`** and
**`CertServer_CLIENT_ASSIGN_BADGES()`**.

## Example

We create an instance of the CertServer with three clients in this
example.

### Instantiation of the Component in CMake

We declare the CertServer in the **`CMakeLists.txt`** as follows:

```CMake
CertServer_DeclareCAmkESComponent(
    MyCertServer
)
```

Assuming that the different clients are actually just instances of the same
component, the following could be used:

```CMake
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        MyCertServer_client
)
```

If there were more instances of the CertServer (e.g., MyCertServer2),
they could be added to the clients with their respective client library
(e.g., MyCertServer2_client).

#### Instantiation and Configuration in CAmkES

Here we show how to set up the component in CAmkES.

#### Declaring the Component

The component is declared like this:

```c
#include "CertServer/camkes/CertServer.camkes"
CertServer_COMPONENT_DEFINE(
    MyCertServer
)
```

#### Instantiating and Connecting the Component

Here we see how to connect the CertServer with an EntropySource and
three clients:

```c
// Instantiate CertServer
component MyCertServer      myCertServer;
// Instantiate storage + entropy
Component RamDisk           storage;
Component EntropySource     entropy;
// Instantiate clients
Component Client            client1;
Component Client            client2;
Component Client            client3;

// Connect interfaces USED by CertServer
CertServer_INSTANCE_CONNECT(
    myCertServer,
    entropy.entropy_rpc,    entropy.entropy_port
)
// Connect interfaces PROVIDED by CertServer
CertServer_INSTANCE_CONNECT_CLIENTS(
    myCertServer,
    client1.server_rpc,     client1.server_port,
    client2.server_rpc,     client2.server_port,
    client3.server_rpc,     client3.server_port
)
```

#### Configuring the Instance

In the configuration example below, the CertServer is set up with a
chain of two certificates:

```c
CertServer_INSTANCE_CONFIGURE(
    myCertServer,
    2,
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDkDCCAnegAwIBAgIUXEphp/RzjJH6jYvDDsBQdrGYRSowDQYJKoZIhvcNAQEL\r\n" \
    "BQAwVzELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\r\n" \
    ...
    "1oHJU/rq1kJljRXXZRO695Uk46IhPNDhAu1L0ml6ansEz9Fl28Yal0Z2+zL0rUAz\r\n" \
    "gqZivg==\r\n" \
    "-----END CERTIFICATE-----\r\n",
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDNzCCAh4CFHbRcbAo6lHOaFjcHj8WsC4A0W+wMA0GCSqGSIb3DQEBCwUAMFcx\r\n" \
    "CzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl\r\n" \
    ...
    "p7KAL9QH/TF7cq73sRSGnRiHlIO+j5jUG25Hcb8IQgSzZ64tF56iGyNhLeieLHTt\r\n" \
    "Ht77hK6Ffknm4lY=\r\n" \
    "-----END CERTIFICATE-----\r\n"
)
```

We use the **`CertServer_CLIENT_ASSIGN_BADGES()`** macro to assign a seL4
badge to the client\'s side of the RPC endpoint.

```c
CertServer_CLIENT_ASSIGN_BADGES(
    client1.server_rpc,
    client2.server_rpc,
    client3.server_rpc
)
```

### Using the Component's Interfaces in C

The following shows how to use the CertServer through its own client library:

```c
// For CertParser API
#include "OS_CertParser.h"

// For client library code of CertServer
#include "CertServer.h"

// For RPC and dataports
#include <camkes.h>

// CertServer client interface as defined in CAmkES
static const if_CertServer_t certServer =
    IF_CERTSERVER_ASSIGN(
        server_rpc,
        server_port);

// This is the cert we want to verify
static const uint8_t certPEM[] =
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDJzCCAg8CAWUwDQYJKoZIhvcNAQELBQAwWDELMAkGA1UEBhMCQVUxEzARBgNV\r\n" \
    "BAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0\r\n" \
    ...
    "UHBt9lKNPJ0Zz1AaYGW0vrE6gHB1Ql+5bNyWnFON5pnvnPBUXA96KPNueA==\r\n" \
    "-----END CERTIFICATE-----\r\n";

int run()
{
    OS_CertParser_VerifyFlags_t flags;

    // Set up an empty chain
    CertServer_initChain(&certServer);

    if ((err = CertServer_addCertToChain(
                     &certServer,
                     OS_CertParserCert_Encoding_PEM,
                     certPEM,
                     sizeof(certPEM)) != OS_SUCCESS)
    {
        Debug_LOG_ERROR("CertServer_addCertToChain() failed with %d", err);
        return -1;
    }

    if ((err = CertServer_verifyChain(&certServer, &flags)) != OS_SUCCESS)
    {
        if (OS_ERROR_GENERIC == err) {
            // This error is returned by the CertParser lib if the actual verification
            // step failed; in that case, the flags value indicates the issue with the
            // chain.
            Debug_LOG_ERROR("Verification of cert failed with flag: %x", flags);
        } else {
            Debug_LOG_ERROR("CertServer_verifyChain() failed with %d", err);
        }
        return -1;
    }

    Debug_LOG_INFO("Verification successful...");

    // Free the chain and the associated cert
    CertServer_freeChain(&certServer);

    return 0;
}
```
