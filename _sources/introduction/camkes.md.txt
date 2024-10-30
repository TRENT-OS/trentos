# CAmkES Framework

While developing systems purely on the bare metal seL4 API is possible,
it is not recommended. Due to the low-level nature of the kernel APIs,
this quickly becomes a time-consuming and error-prone process, in
which largely similar steps with just subtle differences need to be done over
and over again.

This problem is well known and various tools and helper libraries have
been created to make the development easier and automate many low-level
management tasks. The CAmkES framework bundles such a set of tools. It
allows architects to focus on the overall system design and developers
to focus on the implementation of specific system components. All this
can be accomplished without requiring a deep understanding of seL4's
inner working or the API.

CAmkES provides synchronization primitives and component connectors
implementing many common interconnection use cases.

Detailed information about CAmkES can be found as
[https://docs.sel4.systems/projects/camkes](<https://docs.sel4.systems/projects/camkes/>).
An overview of the important CAmkES principles and building blocks is provided
in the CAmkES manual at <https://docs.sel4.systems/projects/camkes/manual.html>.

## Conditional Compilation

Adding C-like a conditional compilation (e.g. **`#if`**, **`#ifdef`** etc.) in
**`.camkes`** files is possible, if the CAmkES processing is using the C
preprocessor. This is enabled by default in the CAmkES CMake files and the SDK
also ensures it is enabled via the following command:

```cmake
set(CAmkESCPP ON CACHE BOOL "" FORCE)
```

Assuming a project has a header file like

```c
...
#define UART_ON    1
#define TEST_LOG   0
...
```

then this can be used in a .camkes file as to enable and disable certain
blocks.

```c
#include "main.h"

#if UART_ON
 connection seL4RPCCall    chanmux_uart        (from chanMux.Output,
                                                to uartDrv.UartDrv);
 connection seL4SharedData uart_dataConnection (from chanMux.outputDataPort,
                                                to uartDrv.inputDataPort);
#endif

#if TEST_LOG
 component Log TestLog;
#endif
```

## Initialization

### System Initialization

- there is no explicit order how components are started, everything must be
  assumed to happen in parallel and in random order;
- since all CAmkES components run in threads, setting the thread priorities ,
  explicitly can enforce an order;
- there is no explicit way how to order the start of components, however, the
  seL4 signalling (emit, wait) mechanism or blocking RPC calls can be used to
  synchronize the startup.

### Component Initialization

- Flow for normal multi-threaded components
  - the CAmkES component main thread calls **`void pre_init(void)`**
  - on return, the main thread wakes up each interface thread, which then
    - calls **`void <interface>__init(void)`**
    - on return, reports init completion to the main thread.
    - blocks until woken up the main thread again.
  - once all interface init threads have reported completion, the main thread
    calls **`void post_init(void)`**
  - on return, the main thread unblocks all interface threads.
  - for components with a control thread, the main thread calls
        **`run(void)`**.
- Flow for single-threaded components
  - there is an implementation for **`pre_init()`**, the following
        macros allow registration of initialization functions, currently
        they are all called from **`pre_init()`** in the given order
    - **` CAMKES_ENV_INIT_MODULE_DEFINE() `**
    - **` CAMKES_PRE_INIT_MODULE_DEFINE() `**
    - **` CAMKES_POST_INIT_MODULE_DEFINE() `**
  - **`post_init()`** should not be implemented, it is reserved
        for future usage by the startup code.
  - **`<interface>__init()`** for specific interfaces if required.
- Notes
  - none of the functions mentioned above must actually be
        implemented, they are called only if implemented.
  - neither **`pre_init()`** nor **`<interface>__init()`** nor
        **`post_init()`** must block.
  - code in **`pre_init()`** runs in a limited CAmkES runtime
        environment, so accesses to other components may not work and
        cause a dead-lock.
  - component internal initialization should not happen in
        **`run()`**, it should happen in **`post_init()`**. Rationale
        is:
    - **`run()`** is executed in parallel to interface
            invocations from other components, which might lead to race
            conditions if the init is still ongoing.
    - a CAmkES component should aim to be passive and use an
            internal state or lock to synchronize the interface
            accesses. There is no need to create a control thread in a
            component that just does initialization and synchronization.

## Component Names and IDs

The name of a component can be obtained during runtime with the function:

```c
const char* get_instance_name(void)
```

There is also a process ID emulation, see
**`<SDK>/sdk-sel4-camkes/tools/camkes/libsel4camkes/include/camkes/pid.h`**.

## Yield

Instead of calling

```c
void seL4_Yield(void)
```

directly, there is

```c
void camkes_sys_sched_yield(void)
```

made available via
**`<SDK>/sdk-sel4-camkes/tools/camkes/libsel4camkes/include/camkes/syscalls.h`**
and implemented in
**`<SDK>/sdk-sel4-camkes/tools/camkes/libsel4camkes/src/sys_yield.c`**.

## Exit

CAmkES components and their threads are not supposed to terminate. For
active components, the **`run()`** function can return an integer value,
but this is currently discarded and the thread is halted. There is also
an internal function

```c
static void sel4_abort(void)
```

in **`<SDK>/sdk-sel4-camkes/tools/camkes/libsel4camkes/src/sys_exit.c`** that
basically suspends the current thread using the seL4 suspend API.

## Notifications, Events and Signals

### Connectors

#### seL4NotificationNative

This is a wrapper around the seL4 notification syscalls. It can be used once the
[CapDL loader](https://docs.sel4.systems/projects/capdl/) has set up all
capabilities, so it works in the **`pre_init()`** and **`post_init()`**
functions.

- Sender: Simple wrapper around syscall **`seL4_Signal()`**
- Receiver: Simple wrapper around syscall **`seL4_Poll()`** or **`seL4_Wait()`**

#### seL4Notification

This is an extended wrapper around the seL4 notification syscall that
supports using a callback function. The receiver function uses a
semaphore internally, thus this connector cannot be used in
**`pre_init()`**  and **`post_init()`**, as its internal semaphore does
not get released.

It uses a secondary thread for receiving interrupts and this thread won't be
allowed to run until after **`post_init()`** returns.

- Sender: wrapper around syscall **`seL4_Signal()`** on the one end.
- Receiver:
  - callback (and one argument) can be registered, will be called
        when the event arrives. Note that the callback is automatically
        unregistered once it got called and must be registered again
        manually
  - there is an internal thread that loops blocking on
        **`seL4_Wait()`** and calls the callback
  - explicit waiting instead of using the callback is also possible,
        but this will block on an internal semaphore then
  - polling is a wrapper around the syscall **`seL4_Poll()`**

### Waiting for Multiple Events Sources

- Use Case:
  - see discussion on the seL4 mailing list
        at [https://lists.sel4.systems/hyperkitty/list/devel\@sel4.systems/thread/ATNQCV25M4NGVM3FHSGSTZHMKVQK24M2](https://lists.sel4.systems/hyperkitty/list/devel@sel4.systems/thread/ATNQCV25M4NGVM3FHSGSTZHMKVQK24M2/)
  - the network stack must wait on 3 different events:
    - timer
    - incoming data from a network
    - outgoing data from an application
- Solution 1:
  - use one **`seL4NotificationNative`** with 3 producers and 1 consumer.
  - The consumer will not know the exact event source, but that might not be
    necessary, one could just check all sources in a well-defined order
- Solution 2:
  - use the **`seL4NotificationQueue`**
- Solution 3
  - use the **`seL4GlobalAsynchCallback`**
- Solution 4:
  - extend **`seL4NotificationNative`** to return the "data word" that is
    returned by **`seL4_Wait()`**.
  - when badges are power-of-two values, all events can be preserved, as when an
    event is signaled, the badge bitwise OR'ed with the notifications data word.

## Mutex and Semaphore

### Mutex

- A mutex locks a code section (e.g. that accesses a shared resource).
- Multiple threads can acquire the mutex, but locking and unlocking should only
  happen pairwise in the same thread.
- The initial state is, that the mutex is available.
- Usage:

```c
has mutex myMutex
```

- Functions
  - Lock: **`<name>_lock()`**
  - Unlock: **`<name>_unlock()`**

### Semaphore

- A counting semaphores is used to lock a countable number of shared resources.
- Requesting and releasing can happen in different threads.
- Initialized to "1" by default, so one shared resource is available.
- Initialization can be customized with **`<name>_value = n`**.
- Usage:

```c
has semaphore mySemaphore
...
mySemaphore_value = 0
```

- Functions
  - Request: **`<name>_wait()`**
  - Release: **`<name>_post()`**

### Binary Semaphore

- A binary semaphore behaves like a mutex but semantically it can be used to
  request and release one shared resource across multiple threads.
- Initialized to "0" by default, so the shared resource is locked.
- Initialization can be customized with **`<name>_value = [0|1]`**.
- Usage:

```c
has binary_semaphore myBinarySemaphore
...
myBinarySemaphore_value = 1
```

- Functions
  - Request: **`<name>_wait()`**
  - Release: **`<name>_post()`**

## RPC Connectors

- Behavior depends on the actual connector that is used for the connection.
- The default assumption should be, that one thread for each interface is used,
  thus explicit synchronization is required.
  - typically, a mutex should be used.
  - thread priorities can be used to get implicit synchronization, but for this
    to work, the whole system must be well designed.
- The connector type **`seL4RPCCall`** uses one thread per interface.
- A connector for the [CakeML](https://cakeml.org/) component exists, that
    has one thread for multiple interfaces.

### Synchronizing RPC Handlers and the Control Thread

In general, CAmkES does not give any guarantees about synchronization
between the control thread (i.e. the code in the **`run()`** function)
and RPC interface handlers. In the current implementation of CAmkES, RPC
handlers are implemented as separate threads, which run independently of
the control thread of a component. Thus RPCs from other components can
arrive while the control thread is executing and the scheduler interrupt
it. Thus RPCs and the control thread must be synchronized explicitly.
Component initialization should not happen in the **`run()`** function,
but preferably in **`pre_init()`** or **`post_init()`**. Both
functions are guaranteed to complete before an PRC handler is invoked.
Note that in **`pre_init()`** no communication with other components is
possible. During runtime, the control thread and RPC interface handler
can be synchronized via semaphores, so no RPC is be processed in
parallel to any other activity. Call **`<sem>_wait()`** in every
handler function on entry and **`<sem>_post()`** when leaving the
handler.

### Sender ID for n:1 RPC Connectors

RPC uses endpoint badges, which can be defined within the CAmkES
configuration file:

```c
<component>.<interface>_attributes = <badge ID>
```

In the RPC handler, one can use this to get the sender endpoint badge
ID:

```c
seL4_Word id = <interface>_get_sender_id();
```

## Multiple Custom Threads in one Component

The closest way to achieve this at the moment is setting this the CAmkES
configuration:

```c
<component>.<instance>.tcb_pool = <num_extra_threads>;
```

During component init, a capability for the TCB can be obtained by
calling:

```c
cap_tcb = camkes_alloc(seL4_TCBObject, 0, seL4_CanRead|seL4_CanWrite);
```

The stack and an IPC buffer for the threads must be allocated manually.

## Dataports

Dataports are the CAmkES abstraction for shared memory. All components
participating in a connection involving dataports get read/write access
to the dataport by default. The default dataport type is **`Buf`** ,
which is implemented as a byte array in C of size **`PAGE_SIZE`** .
Alternatively you can specify a user-defined type for the shared memory
region. Within TRENTOS, the default dataport size is set to 4096 bytes.
More information on dataports in general and different usage examples
are available in the CAmkES manual (see
<https://docs.sel4.systems/projects/camkes/manual.html#an-example-of-dataports>).

## Resources

### Source Code Repositories on Github

- seL4: <https://github.com/seL4>
- CAmkES: <https://github.com/seL4/camkes>

### Manuals

- seL4: <https://sel4.systems/Info/Docs/seL4-manual-latest.pdf>
- CAmkES: <https://docs.sel4.systems/projects/camkes/manual.html>

### Literature

- The seL4 Whitepaper: The seL4 microkernel - An Introduction:
    <https://sel4.systems/About/seL4-whitepaper.pdf>
- seL4 kernel
  - Building trustworthy systems on seL4
        (2019): <https://trustworthy.systems/publications/papers/Kuz_19.abstract>
  - L4 microkernels: The lessons from 20 years of research and deployment
        (2016): <https://trustworthy.systems/publications/nictaabstracts/Heiser_Elphinstone_16.abstract.pml>
  - seL4: Formal verification of an OS kernel
        (2009): <https://trustworthy.systems/publications/nictaabstracts/Klein_EHACDEEKNSTW_09.abstract.pml>
- CAmkES
  - CAmkES glue code semantics (2013): <https://trustworthy.systems/publications/nictaabstracts/Fernandez_GAKK_13:tr.abstract.pml>
  - CAmkES formalization of a component platform
        (2013): <https://trustworthy.systems/publications/nictaabstracts/Fernandez_KKM_13:tr.abstract.pml>
  - capDL: A language for describing capability-based systems
        (2010): <https://trustworthy.systems/publications/nictaabstracts/Kuz_KLW_10.abstract.pml>
  - CAmkES: A component model for secure microkernel-based embedded systems
      (2007): <https://trustworthy.systems/publications/papers/Kuz_LGH_07.abstract>
- Formal verification
  - Formally verified software in the real world (2018): <https://trustworthy.systems/publications/csiroabstracts/Klein_AKMHF_18.abstract.pml>
  - Mathematically verified software kernels: Raising the bar for
        high assurance implementations (2014): <https://trustworthy.systems/publications/nictaabstracts/Potts_BAAKH_14:tr.abstract.pml>
  - Comprehensive formal verification of an OS microkernel
        (2014): <https://trustworthy.systems/publications/nictaabstracts/Klein_AEMSKH_14.abstract.pml>
  - Towards a verified component platform(2013): <https://trustworthy.systems/publications/nictaabstracts/Fernandez_KKA_13.abstract.pml>
  - seL4: From general-purpose to a proof of information flow
        enforcement (2013): <https://trustworthy.systems/publications/nictaabstracts/Murray_MBGBSLGK_13.abstract.pml>
