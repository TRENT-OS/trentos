# CryptoServer

## Overview

The CryptoServer encapsulates key handling (e.g. storing, loading)
and crypto operations (e.g. generation of signatures, etc.) in an
isolated component. It is supposed to be a central element of the
TRENTOS security architecture, usable by other CAmkES components.

**Info:** The CryptoServer uses the KeyStore for the persisting of key
material. Since the KeyStore *at this time* does not support encryption
or integrity protection, it needs to be ensured that keys cannot be
extracted easily (e.g. by using only internal/sealed memories, etc.).

### Concepts

In the following, we introduce the trust model and the idea of "key ownership"
specific to the CryptoServer.

#### Trust Model

The CryptoServer component itself is trusted. It is used to isolate
sensitive data (e.g. keys) in its own component from the rest of the
system, which is potentially not trusted.

#### Ownership Model

Here we shortly describe certain concepts that are introduced in order
to implement the requirements:

- **Component IDs:** In a microkernel system all connections between
    components have a unique ID. Based on this, we can identify the
    sources and sinks of RPC communication.
- **Key name:** Cryptographic keys are stored in the KeyStore under a
    given name. This name is used to address a key when interacting with
    the CryptoServer.
- **Ownership of keys**: The concept of \"key ownership\" means that
    the entity who creates a key (identified by its ID) in the
    CryptoServer (via generate/import) is considered to be its owner and
    is initially the only entity who can load and then use that key.
- **Access rights on keys**: Based on the idea of ownership, a policy
    checker can be implemented which checks whether application X can
    use a key owned by application Y (where X and Y are component IDs).

### Architecture

The CryptoServer makes use of the [Keystore API](../api/key-store_api.md) and
the [Crypto API](../api/crypto_api.md). The CryptoServer uses both internally
and extends them with the ownership concept for keys. Every client of the
CryptoServer has its own instance of the [Crypto API](../api/crypto_api.md) and
KeyStore. This way, there is no way one client can interfere with another
client's keys or cryptographic operations.

In the following diagram, we see that the CryptoServer exposes two
interfaces via a single CAmkES RPC endpoint denoted here as
**`cryptoServer_rpc`**:

1. **`if_OS_Crypto`**: This is the RPC interface internally used by the
    [Crypto API](../api/crypto_api.md); as a result, the CryptoServer can be
    used as RPC server for a [Crypto API](../api/crypto_api.md) instance
    configured in **`OS_Crypto_MODE_CLIENT`** or **`OS_Crypto_MODE_KEY_SWITCH`**
    mode.
2. **`if_CryptoServer`**: This is to be used by the client(s) to explicitly load
    and store keys. The "ownership" concept introduced above applies to this
    interface.

```{mermaid}
flowchart TD
    %% App Node
    subgraph App
        Application(Application)
        CryptoServer_client(CryptoServer_client)
        subgraph CryptoApi_Client["Crypto Library"]
            OS_Crypto_App[OS_Crypto]
            CryptoLibClient[CryptoLibClient]
            CryptoLib_App[CryptoLib]
        end
        Application --> OS_Crypto_App
        Application --> CryptoServer_client
        OS_Crypto_App --> CryptoLibClient
        OS_Crypto_App --> CryptoLib_App
    end

    %% CryptoServer Node
    subgraph CryptoServer
        OS_KeyStore[OS_KeyStore]
        cryptoServer_rpc[cryptoServer_rpc]
        subgraph CryptoApi_Server["Crypto Library"]
            OS_Crypto_Server[OS_Crypto]
            CryptoLib_Server[CryptoLib]
        end
        CryptoServer_client -. "if_CryptoServer" .- cryptoServer_rpc
        cryptoServer_rpc --> OS_Crypto_Server
        cryptoServer_rpc --> OS_KeyStore
        OS_Crypto_Server --> CryptoLib_Server
        CryptoLibClient -. "if_OS_Crypto" .- cryptoServer_rpc
    end
```

Please note that on the side of the application component, the
**`cryptoServer_rpc`** endpoint is never accessed directly but always through
client libraries:

- Functionality related to **`if_OS_Crypto`** is accessed through
    an instance of the [Crypto API](../api/crypto_api.md) library which
    internally implements an RPC client for.
- Functionality related to **`if_CryptoServer`** is accessed through
    **`CryptoServer_client`** module, which internally implements an RPC client
    for.

### Implementation

The CryptoServer maps all client requests to its internal data structure based
on the client's seL4 badge ID. The data structure is effectively an array with
an entry for every client. The badge ID is used as an index into this array, to
retrieve a client's [Keystore API](../api/key-store_api.md) and
[Crypto API](../api/crypto_api.md) context.

The CryptoServer does not allow the exporting of key material through
the [Crypto API](../api/crypto_api.md)'s **`OS_Crypto_Key_export()`** function.
This is to ensure that keys that are generated in the CryptoServer cannot leave
it and thus won't be compromised.

The CryptoServer requires badges/IDs to be in the interval (101-108) and
supports up to 8 clients.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The CryptoServer can be instantiated via the following:

```CMake
CryptoServer_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

In order to use the CryptoServer's client interface, the respective component's
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
        os_crypto
        <NameOfComponent>_client
)
```

Please note that **os_crypto** needs to be added as well, to be able to
use the CryptoServer through the [Crypto API](../api/crypto_api.md).

### Instantiation and Configuration in CAmkES

The CryptoServer offers two interfaces and uses another one, which is
shown here.

#### Declaring the Component

The declaration of the CryptoServer works as follows:

```c
#include "CryptoServer/camkes/CryptoServer.camkes"
CryptoServer_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

#### Instantiating and Connecting the Component

The CryptoServer uses **`if_OS_Entropy`** and **`if_OS_Storage`**, which need to
be set up before connecting any clients:

```c
component <NameOfComponent> <nameOfInstance>;

CryptoServer_INSTANCE_CONNECT(
    <nameOfInstance>,
    <entropy>.<nameOfInterface>, <entropy>.<nameOfDataport>,
    <storage>.<nameOfInterface>, <storage>.<nameOfDataport>
)
```

For its clients, the CryptoServer offers the **`if_OS_Crypto`** and
**`if_CryptoServer`** interfaces, which can be connected to the respective
clients as follows:

```c
CryptoServer_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>,
    <client1>.<nameOfInterface>, <client1>.<nameOfDataport>,
    <client2>.<nameOfInterface>, <client2>.<nameOfDataport>,
    ...
)
```

This list can contain up to 8 clients.

#### Configuring the Instance0

The CryptoServer can be configured for each client and requires two
parameters (unfortunately this cannot yet be done via a macro):

- **`storageLimit:`** sets the limit (in bytes) allowed by a client to be
    stored in its own KeyStore in the CryptoServer.
- **`allowedIds:`** is a list of IDs that can access the (= load) keys
    owned by that particular client. Please note that each list of
    **`allowedIds`** needs to have as many entries as clients are
    connected to the CryptoServer. In order to \"fill up\" the list of
    IDs (in case a client\'s keystore should not be accessible by all
    other clients) the value \"0\" should be added. Per convention,
    client IDs are assigned from 101 to 108, in the order in which
    clients are listed in the
    **`CryptoServer_CLIENT_ASSIGN_BADGES()`** macro.

This config structure needs to have as many entries as there are clients
using the CryptoServer. The first entry in the config struct matches the
first client in the **`CryptoServer_INSTANCE_CONNECT_CLIENTS()`** macro.

```c
<nameOfInstance>.cryptoServer_config = {
    "clients": [
        { "storageLimit": <sizeInBytes>, "allowedIds": [101,102,...] },
        { "storageLimit": <sizeInBytes>, "allowedIds": [...] },
        ...
    ]
};
```

The CryptoServer offers one RPC endpoint, for which the CAmkES badge ID
needs to be assigned explicitly. For this, the following code can be
used:

```c
CryptoServer_CLIENT_ASSIGN_BADGES(
    <client1>.<nameOfInterface>,
    <client2>.<nameOfInterface>,
    ...
)
```

The order of assignment must be the same
for **`CryptoServer_INSTANCE_CONNECT_CLIENTS()`** and
**`CryptoServer_CLIENT_ASSIGN_BADGES()`**.

## Example

We create an instance of the CryptoServer with three clients in this
example.

### Instantiation of the Component in CMake

We declare the CryptoServer in the **`CMakeLists.txt`** as follows:

```CMake
CryptoServer_DeclareCAmkESComponent(
    MyCryptoServer
)
```

Assuming that the different clients are actually just instances of the
same component, the following could be used:

```CMake
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_crypto
        MyCryptoServer_client
)
```

### Instantiation and Configuration in CAmkES

Here we show how to set up the component in CAmkES.

#### Declaring the Component

The component is declared like this:

```c
#include "CryptoServer/camkes/CryptoServer.camkes"
CryptoServer_COMPONENT_DEFINE(
    MyCryptoServer
)
```

#### Instantiating and Connecting the Component

Here we see how to connect the CryptoServer with a
[RamDisk](../components/ram-disk.md) component, an
[EntropySource](../components/entropy-source.md) component and three clients:

```c
// Instantiate CryptoServer
component MyCryptoServer    myCryptoServer;
// Instantiate storage + entropy
Component RamDisk           storage;
Component EntropySource     entropy;
// Instantiate clients
Component Client            client1;
Component Client            client2;
Component Client            client3;

// Connect interfaces USED by CryptoServer
CryptoServer_INSTANCE_CONNECT(
    myCryptoServer,
    entropy.entropy_rpc,    entropy.entropy_port,
    storage.storage_rpc,    storage.storage_port
)
// Connect interfaces PROVIDED by CryptoServer
CryptoServer_INSTANCE_CONNECT_CLIENTS(
    myCryptoServer,
    client1.server_rpc,     client1.server_port,
    client2.server_rpc,     client2.server_port,
    client3.server_rpc,     client3.server_port
)
```

#### Configuring the Instance

The main configuration block needs an entry for each client.  The
specific configuration below allows all three clients to use 16KiB of
KeyStore storage. For the first two clients, only the clients themselves
can access their KeyStores, keys from the KeyStore of the third client can be
accessed by all others.

```c
myCryptoServer.cryptoServer_config = {
    "clients": [
        { "storageLimit": 1024*16, "allowedIds": [101,  0,  0] },
        { "storageLimit": 1024*16, "allowedIds": [102,  0,  0] },
        { "storageLimit": 1024*16, "allowedIds": [101,102,103] },
    ]
};
```

Please note that the order of configuration must correspond to the order
in which the clients appear in the **`CryptoServer_INSTANCE_CONNECT_CLIENTS()`**
macro.

We use the **`CryptoServer_CLIENT_ASSIGN_BADGES()`** macro to assign a seL4
badge to the client\'s side of the RPC endpoint.

```c
CryptoServer_CLIENT_ASSIGN_BADGES(
    client1.server_rpc,
    client2.server_rpc,
    client3.server_rpc
)
```

## Using the Component's Interfaces in C

The following shows how to use the CryptoServer through an instance of the
[Crypto API](../api/crypto_api.md) and through its extra interface (for handling
keys):

```c
// For Crypto API
#include "OS_Crypto.h"

// For client library code of CryptoServer
#include "CryptoServer.h"

// For RPC and dataports
#include <camkes.h>

// CryptoServer client interface as defined in CAmkES
static const if_CryptoServer_t cryptoServer =
    IF_CRYPTOSERVER_ASSIGN(server_rpc);

// Crypto API instance in "CLIENT" mode,
// using the RPC interface (and dataport) as defined in CAmkES
static OS_Crypto_Config_t cryptoCfg =
{
    .mode = OS_Crypto_MODE_CLIENT,
    .rpc = IF_OS_CRYPTO_ASSIGN(
        server_rpc,
        server_port)
};

// Main demo function
int run()
{
    // Declare handles for API and objects
    OS_Crypto_Handle_t hCrypto;
    OS_CryptoKey_Handle_t hKey;
    OS_CryptoCipher_Handle_t hCipher;
    // Declare inputs and outputs for crypto operation
    uint8_t pt[16] = "0123456789abcdef";
    uint8_t ct[16];
    size_t ptLen = 16, ctLen = 16;

    // Set up Crypto API in "CLIENT" mode to
    // automatically switch between local and remote LIB instance
    OS_Crypto_init(&hCrypto, &cryptoCfg);

    // Load key with name "root key" owned by ID=0
    CryptoServer_loadKey(&cryptoServer, &hKey, hCrypto, 0, "root key");

    // Use newly loaded key for some crypto operation
    OS_CryptoCipher_init(&hCipher, hCrypto, hKey, OS_CryptoCipher_ALG_AES_ECB_ENC, NULL, 0);
    OS_CryptoCipher_process(hCipher, pt, ptLen, ct, &ctLen);
    OS_CryptoCipher_free(hCipher);

    // Free key and API if it is no longer needed
    OS_CryptoKey_free(hKey);
    OS_Crypto_free(hCrypto);
}
```

We can see here that the same CAmkES endpoint (**`myCryptoServer_rpc`**) is
referenced to assign the **`if_CryptoServer`** interface to **`cryptoServer`**,
as well as to assign **`if_OS_Crypto`** to **`cryptoCfg.rpc`**.
