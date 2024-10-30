# Object Oriented C
## Scope
The aim of this article is to explain how to get some of the abilities of OO languages in plain C.

OO languages have the following capabilities, among others, that can be someway mimicked in C:

* Encapsulation
* Namespaces
* Multi-instance
* Inheritance
* Initialization
* Templates

We will use for the examples the code of the [LibIO library](https://github.com/TRENT-OS/lib_io) library. In particular we will see how an abstraction of a stream is created and how further specializations of it are added.

!["LibIO Stream Class"](img/object-oriented-c.png)

## Encapsulation
It is generally a good idea to design a C module that works fully on a context, which is a set of variables that define and hold its state. All such variables are collected in a struct, which is exported with a unique type.

For example, from https://github.com/TRENT-OS/lib_io/blob/integration/include/lib_io/InputFifoStream.h

```c
typedef struct InputFifoStream InputFifoStream;
 
struct InputFifoStream
{
    Stream    parent;
    CharFifo  readBuf;
};
```
This module defines a type that is the collector of the variable defining an object of that type. In the case of an InputFifoStream, it is a composition of a Stream and his CharFifo as read buffer.

**⚠ Rule:**  make in every module Foo.c a type Foo that collects the context and export it in Foo.h

## Namespaces
In C there are no namespaces, and this is a pity! Often name collision happens. A good idea is to give all the exported types, variables, functions (etc.) a module prefix, which is the module name. The module name should also be the base type, we could call it "the Class".

For example, the constructor of an object whose type is InputFifoStream:

```c
bool
InputFifoStream_ctor(InputFifoStream* self,
                     size_t readBufSize);
```
**⚠ Rule:** give to all the exported variables and functions of a module a prefix, which is the module name

## Naming schema
* In ModuleName.h : 
    * ModuleName → main type or "Class Name"
    * ModuleName_ExportedType (capital letter is reserved for types)
    * ModuleName_exportedVar, ModuleName_exportedFunc
    * ModuleName_EXPORTED_MACRO
* In ModuleName.c :
    * private names do not need to be prefixed with ModuleName

## Multi-instance
The concept of using the same code on different contexts is what we call multi-instance. It is very easy to achieve, when the context is encapsulated as we have seen. A good idea to make the code uniform is keep using the same name and the position of the context variable. In our example we have chosen the first position and the name "self", to indicate the object on which the code will be executed.

**⚠ Rule:** for every function exported by a class module, the first argument should be the pointer to the variable "self", which has the class type

## Inheritance
Every developer should now what inheritance is. If we define it as the ability of a child to have the same attribute and methods of the parent plus the ability to extend those attributes and methods and to override them (the methods) then we can directly go to the point: how to do it in C?

A very easy idea is about the ability to inherit attributes is to put, in the struct that defines the type of our class, a containment of the parent as first field of the struct. Exactly as in the example above where InputFifoStream has a parent field as first attribute of type Stream. This makes an up-cast a safe operation and we can always pass a specialized version (like InputFifoStream) to a module that just needs to work with a more generic abstraction (like Stream).

For example:

In InputFifoStream.h
```c
#define InputFifoStream_TO_STREAM(self)  (&self->parent)
```
note that this code is perfectly equal to 
```c
#define InputFifoStream_TO_STREAM(self)  ((Stream*)(self))
```
but the second option becomes sneaky when refactoring code.

Supposing we have an AlarmsStreamer.h class that needs just a stream to send and receive alarms to and from a stream in an agnostic way (the stream may be network or serial or even a file! → this is also design for testability).

In our application code:
```c
// uartStream is of type UartStream and is child of stream
// AlarmsStreamer takes a stream in its constructor
 
AlarmsStreamer streamer;
bool ok = AlarmsStreamer_ctor(&streamer, UartStream_TO_STREAM(&uartStream));
```

**⚠ Rule:** when declaring inheritance do not use any fancy macro for object oriented code (OOC). Just place the parent struct as first field in the derived struct and call it 'parent'.

## Methods
The second aspect of inheritance is methods inheritance and override. Let's take a look at how Stream is done.

```c
typedef struct Stream Stream;
 
typedef size_t
(*Stream_WriteT)(Stream* self, char const* buffer, size_t length);
 
typedef size_t
(*Stream_ReadT)(Stream* self, char* buffer, size_t length);
 
typedef size_t
(*Stream_GetT)(Stream* self,
               char* buff,
               size_t len,
               const char* delims,
               unsigned timeoutTicks);
typedef size_t
(*Stream_AvailableT)(Stream* self);
 
typedef void
(*Stream_FlushT)(Stream* self);
 
typedef void
(*Stream_CloseT)(Stream* self);
 
typedef void
(*Stream_DtorT)(Stream* self);
 
typedef struct
{
    Stream_WriteT       write;
    Stream_ReadT        read;
    Stream_GetT         get;
    Stream_FlushT       flush;
    Stream_FlushT       skip;
    Stream_AvailableT   available;
    Stream_CloseT       close;
    Stream_DtorT        dtor;
}
Stream_Vtable;
 
struct Stream
{
    const Stream_Vtable* vtable;
};
```
Stream defines an interface through its vtable. The vtable must be the first field when a Class wants to expose virtual methods that a child can override. Overrides is always possible at run time and takes place likely in the constructor of the child.

Those methods defined in Stream.h are in the end implemented like this:

```c
INLINE size_t
Stream_write(Stream* self, char const* buffer, size_t length)
{
    Debug_ASSERT_SELF(self);
    return self->vtable->write(self, buffer, length);
}
```
As stated in the first paragraph, a module that exports a function must call his function like this Module_function(..). But we see here that for methods that belong to the vtable there is the need to call the static method by dereferencing the vtable. This makes the Stream_write implementation virtual.

**⚠ Rule:** when a class exports virtual methods they must be in a vtable, the vtable pointer must be the first field of the class struct. The methods can be invoked by inline functions that just call the static implementation (like in the last example) .

## Initialization
Correct initialization and de-initialization is very important in programming. Many bugs are triggered just by mistaking this concept.

About this topic the reader should be aware of some basic facts happening above:

vtable variable is a static const → this is because after construction methods will never be modified. Moreover this tells the compiler that it can place it in an opportune place (e.g. non volatile memory addressable area, like for MCUs)
Constructor is a non-virtual, public method → the reason here are pretty obvious, it has to set the vtable pointer
Destructor is a virtual, static method → here instead the reason is not so obvious. It can happen that we build an object in a different context from where the object is destroyed. The context of destruction may not be aware of the underlying specialized type and call the destructor of the parent class. This happens for example when using containers of complex objects where one side "produces" and the other "consumes".

```c
/* Private functions ---------------------------------------------------------*/
 
...
 
static void
dtor(Stream* stream)
{
    DECL_UNUSED_VAR(InputFifoStream* self) = (InputFifoStream*) stream;
    Debug_ASSERT_SELF(self);
 
    CharFifo_dtor(&self->readBuf);
}
 
/* Private variables ---------------------------------------------------------*/
 
static const Stream_Vtable InputFifoStream_vtable =
{
    .read       = read,
    .get        = get,
    .write      = write,
    .available  = available,
    .flush      = flush,
    .skip       = skip,
    .close      = flush,
    .dtor       = dtor
};
 
/* Public functions ----------------------------------------------------------*/
 
bool
InputFifoStream_ctor(InputFifoStream* self, size_t readBufSize)
{
    Debug_ASSERT_SELF(self);
 
    bool retval = true;
 
    if (!CharFifo_ctor(&self->readBuf, readBufSize))
    {
        goto error1;
    }
    self->parent.vtable = &InputFifoStream_vtable;
    goto exit;
 
error1:
    retval = false;
exit:
    return retval;
}
```
**⚠ Rule:** Give to constructor and destructors always the same name (e.g. ctor, dtor)

**⚠ Rule:** Make the vtable variable a static const and assign it at the construction time to the vtable pointer

**⚠ Rule:** Make the destructor a virtual method

Templates
Templates in C are macros "playing" with types. In our example CharFifo is a class that comes from a template FifoT.h. Check it out from https://bitbucket.hensoldt-cyber.systems/projects/SEOS/repos/lib_utils/browse/include/lib_utils/FifoT.h and take a look.

The code for implementing CharFifo will become just:

in .h
```c
#include "FifoT.h"
 
typedef
FifoT_TYPE(char, size_t)
CharFifo;
 
FifoT_DECLARE(char, CharFifo, size_t);
in .c

#include "LibUtil/CharFifo.h"
 
static inline void
char_dtor(char* c)
{
    *c = -1;
}
 
static inline bool
char_ctorCopy(char* target, char const* source )
{
    Debug_ASSERT(target != NULL && source != NULL);
    *target = *source;
    return true;
}
 
 
/* Public functions ----------------------------------------------------------*/
 
FifoT_DEFINE(char, CharFifo, size_t)
```
**⚠ Note:** Despite at a first glance using macros seems great there is some disadvantage. They are hard to write, read, debug and can confuse code analyzers. It is recommended that code inside macros gets well tested before to go in production.

