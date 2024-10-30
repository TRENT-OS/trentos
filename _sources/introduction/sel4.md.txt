# seL4 Microkernel

seL4 is a microkernel that runs on various architectures. It is not an
operating system and does not implement any functionality beyond
primitives like scheduling, inter-process communication, resource
management, and basic interrupt/exception handling. The seL4 kernel
makes very few assumptions about the threads or tasks that are running
on top of it and is agnostic of the programming language they are
written in and their needs in terms of a certain runtime environment.
All the kernel does after initialization is starting an initial bare
metal user-space thread, which must take over the further system
initialization and setup. The seL4 kernel itself does neither implement
any drivers for platform peripherals nor whole subsystems (e.g.
networking), these must be implemented by userland components. seL4
introduces a couple of advanced kernel concepts (e.g. untyped memory,
capabilities in CSpaces, Endpoints Badges) that are quite different from
other kernels, even within the L4 family. These concepts are explained
in detail in the seL4 documentation.

When interacting with the seL4 APIs directly, a good understanding of
these concepts is required, but for many applications, middleware such
us the CAmkES framework can handle these things internally quite well
and hide most of the complexity. Just the concept of endpoints as
terminals for inter-process communication and the assignment of a badge
to endpoints is important to understand when designing and implementing
systems where servers connect to multiple clients and need to identify
them uniquely during the runtime.

More detailed information about the seL4 microkernel can be found
at [https://docs.sel4.systems/projects/sel4](https://docs.sel4.systems/projects/sel4/).
An overview of the most important seL4 internals (e.g. capabilities), as
well as the principles and the syscall API is provided in the seL4
manual.

## The seL4 Foundation

seL4 was open sourced under GPL by General Dynamics C4 Systems in 2014.
Since then it has been maintained by CSIRO\'s Data61 and the community.
Setting up the seL4 Foundation was the next logical step.

The seL4 Foundation is similar to other foundations of open source projects,
such as the Linux Foundation or the RISC-V Foundation. It forms an open,
transparent and neutral organization tasked with the maintenance and growth of
the seL4 ecosystem as well as protecting the seL4 logo and trademark. It brings
together developers of the seL4 kernel, developers of seL4-based components and
frameworks, and those adopting seL4 in real-world systems.

Its focus is on coordinating, directing and standardizing the development of the
seL4 ecosystem in order to reduce barriers to adoption, raising funds for
development activities, and ensuring clarity of verification claims. The seL4
Foundation also provides endorsement of certified services, training and
products.

HENSOLDT Cyber is strongly committed to this approach and hence supports the
seL4 Foundation from the very beginning.

If you are also interested to be part of the Foundation and experience the
benefits of being a member such as:

- strong interaction with other organizations working on and with the seL4
- gain insights on the development roadmap of the seL4
- participate in seL4 Technical Steering Committee

Please visit <https://sel4.systems/Foundation/> for more details about how to
join the seL4 Foundation.

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

### seL4 Related University Courses

- UNSW Advanced Operating Systems:
    <http://www.cse.unsw.edu.au/~cs9242>
- UNSW Advanced Topics in Software Verification:
    [https://www.cse.unsw.edu.au/\~cs4161](https://www.cse.unsw.edu.au/~cs4161/)
- TUM Praktikum Betriebssysteme - seL4 & TRENTOS
    : [https://www.in.tum.de/os/studium-und-lehre/ss21/praktikum-betriebssysteme-sel4-trentos](https://www.in.tum.de/os/studium-und-lehre/ss21/praktikum-betriebssysteme-sel4-trentos/)
