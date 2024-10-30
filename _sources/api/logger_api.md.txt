# Logger API

## Overview

This module is an abstraction of the logging functionality, that allows setting
up a logger server to which clients can send logging entries.

Logger library supports the following features:

- Different clients can add new entries to the log asynchronously
    without corrupting the output.
- Logs can be printed on the console (stdout).
- Logs can be printed to the file.
- Log level filter can be configured both on the client- and
    server-side.
- Log entry consists of the emitter and consumer metadata and log
    message. Log message can be of the maximum length of the default
    data-port size (page size) subtracted by the emitter and consumer
    metadata data structures stored before the log message entry. Any
    entry that exceeds this maximum length will still be logged but
    truncated.
- Each client has a unique ID which is appended to the log entry, and
    optionally a name.
- A custom logging format can be added.

### Concepts

#### Roles

The following roles are distinguished within the API:

- Client - the entity that wants to log the entries and send the log
    messages to the server.
- Server - receives the log entries and forwards them in the
    configured format to the configured outputs.

#### Log Filter

On both the client- and server-side it is possible to configure a filter for the
log level.

If the entry-level is greater than the configured filtering level, it will be
dropped without further processing.

Please note that client-side filtering is done earlier before the entry data is
copied to improve efficiency.

#### Log Format

When the entry is about to be copied to the target directory, it can be
formatted in any given way. The default log format can be found in
**`OS_LoggerFormat`** and consists of:

- consumer id
- consumer name
- date and time
- emitter filtering level
- consumer filtering level
- message

Adapting this default format can be done by overriding the
**`OS_LoggerAbstractFormat_vtable_t::convert`** function. Beware that any
customized format needs to respect the maximum log format buffer size of
**`OS_Logger_FORMAT_BUFFER_SIZE`** as defined in the library since this is the
buffer the final log message will be copied into.

### Architecture

Due to the above-mentioned features, the following design patterns have been
used in this library.

#### The Observer Pattern

Log server contains one or more Views, i.e. a collection of Subjects to which
Observers are connected.

The typical example would be that Log Server has a "ViewA" (SubjectA with two
Observers) with the entries being displayed in the default format
(see **`OSLoggerFormat.h`**) and printed on the console and **`full_log.txt`**
(i.e. the two Observers are console and **`full_log.txt`**), and a "ViewB" with
the entries being displayed in the custom format (e.g. lite one with less data)
stored in the file **`critical_log.txt`**.

Log clients are now mapped to the proper views based on the requirements.

See the below diagram for a better understanding:

```{mermaid}
flowchart TD
    %% Actors
    ClientA1(ClientA1)
    ClientA2(ClientA2)
    ClientB(ClientB)

    %% Server Components
    subgraph Server
        %% View A Components
        subgraph View_A[View A]
            Subject_A(Subject A)
            Observer_A_console(Observer A Console)
            Observer_A_file(Observer A File)
        end

        %% View B Components
        subgraph View_B[View B]
            Subject_B(Subject B)
            Observer_B_file(Observer B File)
        end

        %% Artifacts
        console(console)
        full_log_txt[full log.txt]
        critical_log_txt[critical log.txt]

        %% Relationships within View A
        Observer_A_console -. "update" .- Subject_A
        Observer_A_file -. "update" .- Subject_A
        Observer_A_console -- "default_format" --> console
        Observer_A_file -- "default_format" --> full_log_txt

        %% Relationships within View B
        Observer_B_file -. "update" .- Subject_B
        Observer_B_file -- "lite_format" --> critical_log_txt
    end

    %% Client to Subject Relationships
    ClientA1 -.-> Subject_A
    ClientA2 -.-> Subject_A
    ClientB -.-> Subject_B

    %% Style and Layout Adjustments
    style Server fill:#f9f,stroke:#333,stroke-width:2px;
    style View_A fill:#ccf,stroke:#333,stroke-width:2px;
    style View_B fill:#ccf,stroke:#333,stroke-width:2px;
```

#### Emitter - Consumer Pairs

The Client-Server model is implemented by the introduction of log entry
emitters and consumers.

```{mermaid}
flowchart TB
    %% Emitter Nodes
    EmitterA1[Emitter A1]
    EmitterA2[Emitter A2]
    EmitterB[Emitter B]

    %% Queue Nodes
    bufferA1[queue bufferA1]
    bufferA2[queue bufferA2]
    bufferB[queue bufferB]

    %% Server and Internal Components
    subgraph Server
        emit[emit]
        subgraph Consumers
            ConsumerA1[Consumer A1]
            ConsumerA2[Consumer A2]
            ConsumerB[Consumer B]
        end
        subgraph Subjects
            SubjectA[Subject A]
            SubjectB[Subject B]
        end
    end

    %% Connections
    EmitterA1 -. "up" .- emit
    EmitterA2 -- "do" --> emit
    EmitterB -. "up" .- emit

    ConsumerA1 -.-> SubjectA
    ConsumerA2 -.-> SubjectA
    ConsumerB -.-> SubjectB

    EmitterA1 --> bufferA1
    EmitterA2 --> bufferA2
    EmitterB --> bufferB

    bufferA1 --> ConsumerA1
    bufferA2 --> ConsumerA2
    bufferB --> ConsumerB

    %% Style and Layout Adjustments
    style Consumers fill:#f9f,stroke:#333,stroke-width:2px
    style Subjects fill:#ccf,stroke:#333,stroke-width:2px
```

##### Emitter

If an Emitter wants to log a new entry, it copies the data to the
exchange buffer (Client-Server shared memory) and it uses the RPC call
**`emit`** .

##### Consumer

On the server-side, there exists a list of consumers. Each consumer is
assigned to one client (1-to-1 consumer-emitter pair).

When **`emit`** was called, a Server iterates over the consumer list and
finds the one that corresponds to the emitter. The selected consumer
processes and forwards the log entry to the corresponding Subject.

### Implementation

For the logging system being able to work, the log server must be
configured properly and has to be connected to the clients.

Each client emits an entry and the server does the proper filtering and
dispatching of messages to the pre-configured outputs.

## Usage

Create a server component that links the logger library, and also link
the logger library to the desired clients.

**Info:** There is no separate build configuration of the library for
the client and server.

Later connect the server and clients with the **`if_OS_Logger`** interface, and
configure the server.

### Client Usage

On the client-side, the logger library requires a call to
**`OS_LoggerEmitter_getInstance()`** for initialization, with a reference to a
filter instance, shared buffer, and the emit RPC.

**Info:** The filter is optional and can be NULL if no filtering is desired.

After the initialization, the system is ready for logging. For logging, the
macros from **`LibDebug/Debug.h`** shall be used.

### Server Usage

On the server-side, there is much more to initialize.

Important to know is, that each client needs an instance of the consumer on the
server-side with:

1. Reference to the shared buffer.
2. Optional filter - pass a NULL pointer if not needed.
3. Reference to the consumer callback.
4. Reference to the **`Subject`** which is connected to the desired views.
5. Optional file - pass a NULL pointer if not needed.
6. An ID of the client.
7. Desired client's name.

So the order of initialization shall be the following:

- STEP1: Initialize the Chain of Consumers.
- STEP2: Create the View consisting of the Subject, Format, and the Output.
- STEP3: Initialize the filter (optional step).
- STEP4: Initialize the Consumer callback.
- STEP5: Initialize the Consumer.
- STEP6: Append the Consumer to the Chain.
- STEP7: Initialize the log file (optional step).

Once the server is initialized, there is nothing more to be done. The
server will wait for the incoming **`emit`** RPC calls and serve them.

The **`demo_iot_app`** integrates the logging system with a client and a server
implementation. Please refer to this as a complete example implementation.

### Declaration of API library in CMake

The libraries **`os_core_api`**, **`os_logger`** and **`lib_debug`** need to be
linked to the component.

## Example

### Instantiation of API in CMake

```CMake
DeclareCAmkESComponent(
    Foo
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_core_api
        lib_debug
        os_logger
        ...
)
```

### Using the API in C

#### Client

```c
#include "lib_debug/Debug.h"

static OS_LoggerFilter_Handle_t filter;

void post_init()
{
    if(NO_FILTER)
    {
        OS_LoggerEmitter_getInstance(logServer_buf, NULL, API_LOG_SERVER_EMIT);
    }
    else
    {
        OS_LoggerFilter_ctor(&filter, log_lvl);
        OS_LoggerEmitter_getInstance(logServer_buf, &filter, API_LOG_SERVER_EMIT);
    }
}

void run()
{
    Debug_LOG_INFO("Hello, world!");
}
```

#### Server

```c
#include "Logger/Server/OS_LoggerFile.h"

#include "Logger/Server/OS_LoggerConsumerChain.h"
#include "Logger/Server/OS_LoggerConsumer.h"

#include "Logger/Server/OS_LoggerOutputConsole.h"
#include "Logger/Server/OS_LoggerOutputFileSystem.h"

#include "Logger/Client/OS_LoggerEmitter.h"

#include "OS_FileSystem.h"

// Required objects for the server to operate correctly.
static OS_LoggerConsumer_Handle_t       consumer;
static OS_LoggerConsumerCallback_t      log_consumer_callback;
static OS_LoggerFilter_Handle_t         filter;

static OS_LoggerFormat_Handle_t         format;
static OS_LoggerSubject_Handle_t        subject;
static OS_LoggerOutput_Handle_t         filesystem, console;

static OS_LoggerFile_Handle_t           logFile;

void pre_init()
{
    // STEP1: Initialize the Chain of Consumers.
    OS_LoggerConsumerChain_getInstance();

    // STEP2: Create the View consisting of the Subject, Format, and the Output.
    OS_LoggerFormat_ctor(&format);
    OS_LoggerSubject_ctor(&subject);

    OS_LoggerOutputFileSystem_ctor(&filesystem, &format);
    OS_LoggerOutputConsole_ctor(&console, &format);

    // Subject is now connected with the Output
    // i.e. View initialization is done.
    OS_LoggerSubject_attach(
        (OS_LoggerAbstractSubject_Handle_t*)&subject,
        &filesystem);

    OS_LoggerSubject_attach(
        (OS_LoggerAbstractSubject_Handle_t*)&subject,
        &console);

    // STEP3: Initialize the filter (optional step).
    OS_LoggerFilter_ctor(&filter, Debug_LOG_LEVEL_DEBUG);

    // STEP4: Initialize the Consumer callback.
    OS_LoggerConsumerCallback_ctor(
        &log_consumer_callback,
        API_LOG_SERVER_GET_SENDER_ID,
        get_time_sec);

    // STEP5: Initialize the Consumer.
    OS_LoggerConsumer_ctor(
       &consumer,
       buffer,                 // 1. Reference to the shared buffer.
       &filter,                // 2. Pointer to the filter (can be NULL if not needed).
       &log_consumer_callback, // 3. Reference to the consumer callback.
       &subject,               // 4. Reference to the `Subject`.
       &logFile,               // 5. Reference to the file (can be NULL if not needed).
       id,                     // 6. Client's id.
       "Emitter_Name"          // 7. Client's name
    );

    // STEP6: Append the Consumer to the Chain.
    OS_LoggerConsumerChain_append(&consumer);

    // STEP7: Initialize the log file (optional step).
    OS_LoggerFile_ctor(&logFile, hFs, "Log_File_Name");
    OS_LoggerFile_create(&logFile);
}

void run()
{
        /* Not needed as server is event driven */
}
```
