# EntropySource

## Overview

The EntropySource component implements a platform-specific driver for
providing entropy from a hardware-based true random number generator
(TRNG) and offers access to this hardware via a defined interface.

A good source of entropy is critical for any application that wants to
use cryptography. The [Crypto API](../api/crypto_api.md) specifically requires
"good random numbers" for a variety of operations:

- To provide the user with access to random data
- To generate keys
- To protect against side-channel attacks

Especially the last two use cases depend on a good source of entropy.

**Info:** Currently, TRENTOS does not provide a platform-specific driver
and simply writes **`0xff`** into the buffer provided by the
interface\'s user.

### Implementation

The EntropySource component offers the **`if_OS_Entropy`** interface; other
components can use this interface to request raw entropy, which
typically would be fed into a deterministic random number generator
(DRBG).

The implementation of this component is specific to each target
platform, depending on the availability of actual hardware modules which
can act as a source of entropy.

**Info:** Consequently, the EntropySource contains a simple dummy
implementation which can be overridden by the user. For this, a custom
implementation of the driver\'s interface needs to be provided, which
can be passed into the build process of the EntropySource via CMake
parameters (see below).

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The EntropySource provides an implementation of the **`if_OS_Entropy`**
interface. However, the instantiation of the component in the
**`CMakeLists.txt`** allows to override the dummy implementation with an actual
driver, by passing optional parameters which override the parameters of the
dummy:

```CMake
EntropySource_DeclareCAmkESComponent(
    <NameOfComponent>
    INCLUDES
        <DriverIncludes>
    SOURCES
        <DriverSource>
)
```

We see here that the parameters correspond to the typical way of using
CMake.

### Instantiation and Configuration in CAmkES

Typically, the EntropySource is instantiated and connected to a single
client for use with the [Crypto API](../api/crypto_api.md).

#### Declaring the Component

This is how the EntropySource is declared:

```c
#include "EntropySource/camkes/EntropySource.camkes"
EntropySource_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

#### Instantiating and Connecting the Component

This is how a client is connected to the EntropySource via
the **`if_OS_Entropy`** interface:

```c
component <NameOfComponent>   <nameOfInstance>

EntropySource_INSTANCE_CONNECT_CLIENT(
    <nameOfInstance>,
    <client>.<nameOfInterface>, <client>.<nameOfDataport>
)
```

## Example

Here we show how to instantiate the EntropySource with a custom driver
and use it in a component with the [Crypto API](../api/crypto_api.md).

### Instantiation of the Component in CMake

The EntropySource dummy implementation is overridden by a custom C file passed
along in the **`CMakeLists.txt`**:

```CMake
EntropySource_DeclareCAmkESComponent(
    MyEntropySource
    INCLUDES
        include/
    SOURCES
        src/platform_specific_driver.c
)
```

### Instantiation and Configuration in CAmkES

#### Declaring the Component

This is how the EntropySource is instantiated:

```c
#include "EntropySource/camkes/EntropySource.camkes"
EntropySource_COMPONENT_DEFINE(
    MyEntropySource
)
```

#### Instantiating and Connecting the Component

This is how the EntropySource is connected to a client, who has to use
the **`if_OS_Entropy`** interface.

```c
// Instantiate EntropySource
component MyEntropySource   entropySource;
// Instantiate client
component Client            client;

// Connect interface PROVIDED by EntropySource
EntropySource_INSTANCE_CONNECT_CLIENT(
    entropySource,
    client.myEntropy_rpc, client.myEntropy_port
)
```

### Using the Component\'s Interfaces in C

The user of the EntropySource can simply pass the RPC functions to the
[Crypto API](../api/crypto_api.md) which takes care of the rest:

```c
// For Crypto API
#include "OS_Crypto.h"

// For the CAmkES interface of the EntropySource
#include <camkes.h>

static OS_Crypto_Config_t cfg =
{
    .mode = OS_Crypto_MODE_LIBRARY_ONLY,
    .library.entropy = IF_OS_ENTROPY_ASSIGN(
        myEntropy_rpc,
        myEntropy_port),
};

...

int run() {
    ...
    OS_Crypto_init(&hCrypto, &cfg);
    ...
}
```
