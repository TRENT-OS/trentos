# TimeServer

## Overview

The TimeServer uses a hardware timer and gives its clients access to one
or more timers based on this hardware timer.

### Implementation

The TimeServer uses the platform-specific timer modules from the seL4
platform support library. This component allows multiple clients to
share a hardware timer for common functions such as sleep, periodic
notifications or getting a timestamp. Each client component can have up
to four timers, which can be used for one purpose at a time only.

The TimeServer offers the **`if_OS_Timer`** CAmkES interface, but for ease
of use it also supports a client library which wraps a subset of the
CAmkES interface's functionality.

## Usage

This is how the component can be instantiated in the system.

### Declaration of the Component in CMake

The TimeServer can be declared via a simple macro in
the **`CMakeLists.txt`** file:

```CMake
TimeServer_DeclareCAmkESComponent(
    <NameOfComponent>
)
```

If a component wants to use the client interface of the TimeServer, the
component needs to add **`<NameOfComponent>_client`** to its LIBS parameter;
it can then include **`TimeServer.h`** and use the simplified client API
instead of accessing the RPC interface directly.

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

In order to wire the TimeServer to a set of clients, it has to be
declared, instantiated, connected and configured in the main CAmkES
composition of the system.

#### Declaring the Component

The component is simply declared in the main CAmkES file by using its
respective macro:

```c
#include "TimeServer/camkes/TimeServer.camkes"
TimeServer_COMPONENT_DEFINE(
    <NameOfComponent>
)
```

This declares a component type (i.e., a server), which can be instantiated
multiple times later. The **`<NameOfComponent>`** must correspond to the name
chosen in the **`CMakeLists.txt`** file, where the source code of the server is
defined.

#### Instantiating and Connecting the Component

The following macro allows creating an instance of the TimeServer
component (i.e., the server) and connects it to one (or more) clients.
These clients have to use the **`if_OS_Timer`** interface (see
**`<nameOfInterface>`**) and a signal sink (see **`<nameOfEvent>`**) to
interact with the TimeServer instance.

```c
component <NameOfComponent> <nameOfInstance>;

TimeServer_INSTANCE_CONNECT_CLIENTS(
    <nameOfInstance>,
    <client1>.<nameOfInterface>, <client1>.<nameOfEvent>,
    <client2>.<nameOfInterface>, <client2>.<nameOfEvent>,
    ...
)
```

Please note that up to 8 clients can be connected like this.

#### Configuring the Instance

The TimeServer is configured to allow up to four timers per client. To
change this value, the following parameter can be set:

- **`timers`:** an integer indicating the number of timers a single
    client can use

The full macro looks like this:

```c
TimeServer_INSTANCE_CONFIGURE(
    <nameOfInstance>,
    <timers>
)
```

#### Assigning Clients' Badges

The TimeServer uses the "badge" assigned by seL4 to each client's RPC
endpoint to manage client-specific states (e.g., timers). Badges can be
assigned via this macro:

```c
TimeServer_CLIENT_ASSIGN_BADGES(
    <client1>.<nameOfInterface>,
    <client2>.<nameOfInterface>,
    ...
)
```

Please note that the order of clients needs to correspond to the
order **`TimeServer_INSTANCE_CONNECT_CLIENTS()`** macros. The TimeServer can
handle up to 8 clients.

## Example

In the following example, we have one instance of the TimeServer and one
client using it to sleep for a certain time.

### Instantiation of the Component in CMake

The component is added to the system as MyTimeServer:

```CMake
TimeServer_DeclareCAmkESComponent(
    MyTimeServer
)
```

Additionally, we would like to use the simplified interface so we add
the specific client library to the component that wants to use the
TimeServer instance:

```CMake
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        MyTimeServer_client
)
```

### Instantiation and Configuration in CAmkES

The component needs to be declared and connected to its respective
clients.

#### Declaring the Component

The component is declared as MyTimeServer in the main CAmkES file. The
name must correspond to the name used above in the **`CMakeLists.txt`**
file:

```c
#include "TimeServer/camkes/TimeServer.camkes"
TimeServer_INSTANCE_CONFIGURE(
    MyTimeServer
)
```

#### Instantiating and Connecting the Component

The following code fragment instantiates the TimeServer and connects it
to the clients:

```c
// Instantiate TimeServer component
component MyTimeServer      myTimeServer;
// Instantiate two clients
component Client            client1;
component Client            client2;

// Connect interfaces PROVIDED by the TimeServer
TimeServer_INSTANCE_CONNECT_CLIENTS(
    myTimeServer,
    client1.timer_rpc, client1.timer_notify
    client2.timer_rpc, client2.timer_notify
)
```

In this example, the clients are using names **`timer_rpc`** for its local
RPC endpoint and **`timer_notify`** for the respective signal sink.

#### Assigning Clients' Badges

We assign the badges to the clients' RPC endpoints via this macro:

```c
TimeServer_CLIENT_ASSIGN_BADGES(
    client1.timer_rpc,
    client2.timer_rpc
)
```

### Using the Component's Interfaces in C

If the component that wants to use the TimeServer has included the
respective client library, we can use the simplified interface in the C
code, instead of calling the CAmkES RPC interface directly.

Please note how the endpoint name and signal sink are referred when
assigning the CAmkES interface via **`IF_OS_TIMER_ASSIGN()`**.

```c
// For TimeServer client wrapper
#include "TimeServer.h"

// For the CAmkES generated interface
#include <camkes.h>

// Assign the RPC endpoint based on the names used by this client
static const if_OS_Timer_t timer =
    IF_OS_TIMER_ASSIGN(
        timer_rpc,
        timer_notify);

...

int run() {
    ...
    // Sleep for one second
    TimeServer_sleep(&timer, TimeServer_PRECISION_SEC, 1);
    ...
}
```
