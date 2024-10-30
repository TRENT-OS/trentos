# KeyStore API

## Overview

The Keystore API allows to store and load keys identified by memorable names.
The keys might be created or used by the [Crypto API](crypto_api.md).

The Keystore offers a range of functions for the handling of keys (defined in
**`OS_Keystore.h`**):

- store key,
- load key,
- delete key,
- copy key from one Keystore to another,
- move key from one Keystore to another,
- wipe the Keystore.

The Keystore API is not limited to a fixed number implementations but is
designed to be extensible with additional implementations. The TRENTOS SDK
already contains two different Keystore implementations:

- KeystoreFile which uses a file based storage backend and
- KeystoreRamFV which stores the keys in RAM and whose code is
    **formally verified**.

The class diagram below shows the current implementations available with their
dependencies. KeystoreFile has a dependency on a FileSystem handle
(to store/load keys) and a Crypto handle (to hash the keys) while KeystoreRamFV
has no dependency.

```{mermaid}
classDiagram
    class OS_Keystore {
        +free()
        +storeKey()
        +loadKey()
        +deleteKey()
        +copyKey()
        +moveKey()
        +wipeKeystore()
    }
    class OS_KeystoreFile {
        +init()
    }
    class OS_KeystoreRamFV {
        +init()
    }

    class OS_FileSystem
    note for OS_FileSystem "Store/load the keys"

    class OS_Crypto
    note for OS_Crypto "Hash the keys"

    OS_Keystore <|-- OS_KeystoreFile : implements
    OS_Keystore <|-- OS_KeystoreRamFV : implements
    OS_KeystoreFile o-- OS_FileSystem : uses
    OS_KeystoreFile o-- OS_Crypto : uses
```

The Keystore implementations are separate libraries which must be linked
into the application that wants to use them. Individual initialization
routines for the different implementations need to be called by the user
application before the Keystore API can be used. The init functions are
designed to return an  **`OS_Keystore_Handle_t`** value (an abstract
reference to the object initialized) that can be used with the Keystore
API.

**Info:** Dependent on the support for encryption or integrity
protection of a certain Keystore implementation it needs to be checked that keys
cannot be extracted and/or corrupted (e.g. by using only internal/sealed
memories).

The following chapter will give more detailed information about the
included Keystore implementations.

## Implementations

### KeystoreFile

KeystoreFile is an implementation that performs storage by using an instance of
the [FileSystem API](file-system_api.md), it writes opaque buffers of
cryptographic material (as exported from the [Crypto API](crypto_api.md)) into a
file, for which the KeystoreFile needs a handle to a mounted file system.
Conversely, it can also load keys back into memory.

Every KeystoreFile instance has an "instance name" (set during the
instantiation of the KeystoreFile), which allows having several
instances of the KeystoreFile on the same file system. Each key is
stored in its own file and the file name is constructed from a
Keystore's "instance name" and the respective "key name".

**Info:** Using different instances of the KeystoreFile with the same file
system requires each instance to have a unique instance name. Otherwise, these
instances might interfere with each other\'s files.

**Info:** The isolation between two KeystoreFile instances using the
same piece of storage and the same file system is weak. If one instance
needs to be separated from another instance, each instance should have
its own piece of storage (disjunctive ranges of storage can be assigned
via the StorageServer).

#### Usage

This is how the API can be instantiated in the system.

##### Declaration of API Library in CMake

There is a single build target for using the Keystore API with the
KeystoreFile implementation, which needs to be included for every
component that wants to use it.

- **os_keystore_file:** Generic build target to include the
    KeystoreFile implementation

Additionally, the KeystoreFile will require crypto and file system, so
the user must include the respective build targets as well.

#### Example

Here we show how to use the Keystore API with a simple example.

##### Instantiation of API in CMake

Here we see how a component can use the build target of the library to
include it (besides crypto and file system):

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
        os_filesystem
        os_keystore_file
)
```

##### Using the API in C

In the following example, we show how to set up the Keystore API with
the KeystoreFile implementation and a file system and crypto instance,
how to load a key (assuming it exists) and how to import it into the
crypto instance.

Many implementation aspects of this example (configs of crypto, file
system) have been omitted. Please refer to the respective chapters in
the handbook for further information. Please also note that you
need to include **`OS_KeystoreFile.h`** in the source file in which you call
**`OS_KeystoreFile_init()`**. The init function of the Keystore is
implementation specific and therefore not part of the Keystore API.

```c
#include "OS_KeystoreFile.h"

...

int run()
{
    OS_FileSystem_Handle_t hFs;
    OS_Crypto_Handle_t hCrypto;
    OS_Keystore_Handle_t hKeys;
    OS_CryptoKey_Data_t data;
    OS_CryptoKey_Handle_t hMyKey;

    // Set up filesystem
    OS_FileSystem_init(&hFs, &cfgFs);
    OS_FileSystem_mount(hFs);

    // Set up crypto instance and keystore
    OS_Crypto_init(&hCrypto, &cfgCrypto);

    OS_KeystoreFile_init(&hKeys, hFs, hCrypto, "myKeystore");

    // Load a key with name "myKey" from keystore and import it into crypto instance
    OS_Keystore_loadKey(hKeys, "myKey", &data, &dataLen);
    OS_CryptoKey_import(&hMyKey, hCrypto, &data);

    // Use key for crypto operations
    ...
}
```

### KeystoreRamFV

KeystoreRamFV is a formally verified implementation that performs
storage in a RAM. The implementation expects to receive a pointer to a memory
buffer and the buffer size. The macro
**`OS_KeystoreRamFV_SIZE_OF_BUFFER(num_elements)`** helps to convert the desired
number of keys to the appropriate buffer size.

**Info:** This implementation stores the keys without
additional hashing.

**Info:** There is no persistence of the keys after a power cycle or
after an **`init()`** - **`free()`** cycle.

**Info:** The formal verification (done with Isabelle theorem proof
assistant, see <https://isabelle.in.tum.de/>) has been performed on the
Keystore implementation core functions. Additional information is
available from HENSOLDT Cyber on request.

#### Usage

This is how the API can be instantiated in the system.

##### Declaration of API Library in CMake

There is a single build target for using the Keystore API with the
KeystoreRamFV implementation, which needs to be included for every
component that wants to use it.

- **os_keystore_ram_fv:** Generic build target to include the
    KeystoreRamFV implementation.

#### Example

Here we show how to use the Keystore API with a simple example.

##### Instantiation of API in CMake

Here we see how a component can use the build target of the library to
include it:

```c
DeclareCAmkESComponent(
    Client
    SOURCES
        ...
    C_FLAGS
        ...
    LIBS
        ...
        os_keystore_ram_fv
)
```

##### Using the API in C

In the following example, we show how to set up the Keystore API with
KeystoreRamFV implementation, how to generate a key with the crypto
instance, store the key and load it later with the Keystore and use the
key to encrypt some text with the crypto instance.

Many implementation aspects of this example (configs of crypto) have
been omitted. Please refer to the respective chapters in the handbook
for further information. Please also note that you need to include
**`OS_KeystoreRamFV.h`** in the source file in which you call
**`OS_KeystoreRamFV_init()`**. The init function of the Keystore is
implementation specific and therefore not part of the Keystore API.

```c
#include "OS_KeystoreRamFV.h"

...

// Let's say we will need to store 10 keys
#define KEYSTORE_NUM_ELEMENTS 10

// The following is to define the size of the RAM buffer that we will use to
// initialize the KeystoreRamFV object.
// By using the macro OS_KeystoreRamFV_SIZE_OF_BUFFER() we can obtain the amount
// of memory in bytes starting from the number of elements.
#define KEYSTORE_RAM_BUF_SIZE OS_KeystoreRamFV_SIZE_OF_BUFFER(KEYSTORE_NUM_ELEMENTS)

#define PLAIN_TEXT \
\
"\"O frati\", dissi, \"che per cento milia \
perigli siete giunti a l'occidente, \
a questa tanto picciola vigilia \
\
d'i nostri sensi ch'e' del rimanente \
non vogliate negar l'esperienza, \
di retro al sol, del mondo sanza gente. \
\
Considerate la vostra semenza: \
fatti non foste a viver come bruti, \
ma per seguir virtute e canoscenza\""

static const OS_CryptoKey_Spec_t aes256Spec =
{
    .type = OS_CryptoKey_SPECTYPE_BITS,
    .key = {
        .type = OS_CryptoKey_TYPE_AES,
        .attribs.keepLocal = true,
        .params.bits = 256
    }
};

int run()
{
    OS_Crypto_Handle_t hCrypto;
    OS_Keystore_Handle_t hKeys;
    OS_CryptoKey_Data_t data;
    OS_CryptoKey_Handle_t hMyKey;
    OS_CryptoCipher_Handle_t hCipher;

    char keystoreRamBuf[KEYSTORE_RAM_BUF_SIZE];
    char bufEncText[sizeof(PLAIN_TEXT)] = {0};
    size_t sizeEncText = 0;

    // Set up crypto instance and keystore
    OS_Crypto_init(&hCrypto, &cfgCrypto);
    OS_KeystoreRamFV_init(&hKeys, keystoreRamBuf, KEYSTORE_RAM_BUF_SIZE);

    // Generate an AES key
    OS_CryptoKey_generate(&hMyKey, hCrypto, &aes256Spec);

    // Export the key data into a temporary variable
    OS_CryptoKey_export(hMyKey, &data);

    // Store the key with name "myKey" to keystore
    OS_Keystore_storeKey(hKeys, "myKey", &data, sizeof(data));

    // Application performs something
    ...

    // Load the key with name "myKey" from keystore and import it into crypto instance
    OS_Keystore_loadKey(hKeys, "myKey", &data, &dataLen);
    OS_CryptoKey_import(&hMyKey, hCrypto, &data);

    // Encrypt the plaint text
    sizeEncText = sizeof(bufEncText);
    OS_CryptoCipher_init(&hCipher, hCrypto, hMyKey, OS_CryptoCipher_ALG_AES_ECB_ENC, NULL, 0);
    OS_CryptoCipher_process(hCipher, PLAIN_TEXT, sizeof(PLAIN_TEXT), bufEncText, &sizeEncText);
    OS_CryptoCipher_free(hCipher);

    // Application performs something
    ...
}
```
