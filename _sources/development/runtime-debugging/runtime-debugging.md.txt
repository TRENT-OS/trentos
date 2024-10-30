# Runtime Debugging

## Subpages

```{toctree}
:caption: Subpages
:maxdepth: 1

debugging-qemu
debugging-hw-platforms_open-ocd
debugging-hw-platforms_segger
```

## Introduction

The purpose of this page is to give an overview of the current state of
the explored runtime debugging setups for TRENTOS on the supported
platforms. The main focus of the page will be on three different setups:

- QEMU + **`gdbgui`** → A free open source toolchain that does not
    require any HW and uses the QEMU emulator together with the
    **`gdbgui`** visual debugger
- FT232H + OpenOCD + **`gdbgui`** → A free open source toolchain that
    does not require any special HW apart from the (cheap) Adafruit
    FT232H breakboard based on the FTDI chip available here:
    <https://www.adafruit.com/product/2264>
- Segger J-Link Base Compact + Ozone → A toolchain from Segger which
    uses the J-Link Base Compact debug probe
    (<https://www.segger.com/products/debug-probes/j-link/models/j-link-base/>)
    together with their proprietary SW Ozone
    (<https://www.segger.com/products/development-tools/ozone-j-link-debugger/>).\
    **Note:** Depending on the probe used, Ozone might require a license
    to use (see:
    <https://www.segger.com/products/development-tools/ozone-j-link-debugger/#ozone-licensing>).
    Additionally, a setup using VS Code (instead of Ozone) together with
    the J-Link probe is showcased.

## Overview of the Functionalities

| Symbol                        | Explanation                                                |
|-------------------------------|------------------------------------------------------------|
| !["tick"](img/check.svg)      | The feature works reliably.                                |
| !["warning"](img/warning.svg) | The feature works to an extent / under special conditions. |
| !["error"](img/error.svg)     | The feature does not work.                                 |

### Debugging Applications Running in QEMU

| Functionality                           | QEMU                          |
|-----------------------------------------|-------------------------------|
| Halting/Releasing the program execution | !["tick"](img/check.svg)      |
| Setting/Hitting breakpoints             | !["tick"](img/check.svg)      |
| Resetting the platform                  | !["warning"](img/warning.svg) |
| Stepping through                        | !["tick"](img/check.svg)      |
| Memory inspection                       | !["tick"](img/check.svg)      |
| Register querying                       | !["tick"](img/check.svg)      |

### Debugging Applications Running on HW Platforms

#### OpenOCD Toolchain

| Functionality                           | RPI3B+                                                     | Nitrogen6_SoloX               | BD-SL-i.MX6                   |
|-----------------------------------------|------------------------------------------------------------|-------------------------------|-------------------------------|
| Halting/Releasing the program execution | !["warning"](img/warning.svg)                              | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Setting/Hitting breakpoints             | !["warning"](img/warning.svg)                              | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Resetting the platform                  | !["error"](img/error.svg)                                  | !["warning"](img/warning.svg) | !["warning"](img/warning.svg) |
| Stepping through                        | !["error"](img/error.svg)                                  | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Memory inspection                       | !["tick"](img/check.svg)<br><br>(if halting is successful) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Register querying                       | !["tick"](img/check.svg)<br><br>(if halting is successful) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |

#### Segger Toolchain

| Functionality                           | RPI3B+                    | Nitrogen6_SoloX               | BD-SL-i.MX6                   |
|-----------------------------------------|---------------------------|-------------------------------|-------------------------------|
| Halting/Releasing the program execution | !["error"](img/error.svg) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Setting/Hitting breakpoints             | !["error"](img/error.svg) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Resetting the platform                  | !["error"](img/error.svg) | !["warning"](img/warning.svg) | !["warning"](img/warning.svg) |
| Stepping through                        | !["error"](img/error.svg) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Memory inspection                       | !["error"](img/error.svg) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |
| Register querying                       | !["error"](img/error.svg) | !["tick"](img/check.svg)      | !["tick"](img/check.svg)      |

## Additional Notes

- As it is clear from the tables presented in the previous sections,
    runtime debugging has the most limited scope on the RPi3B+ platform
    since it is not supported by the Segger toolchain, and a limited set
    of features is available when using the OpenOCD toolchain. Still,
    the setup for this platform is included in the documentation to
    serve as a starting point to be expanded upon by more experienced
    users or as an additional tool (with limited functionalities) which
    can be of use in certain circumstances.
- Even though the feature tables present a similar story when it comes
    to using the OpenOCD toolchain compared to the Segger toolchain with
    the Nitrogen6_SoloX and BD-SL-i.MX6 platforms, the user experience
    is quite different. The benefits of the Segger toolchain are
    primarily the convenience of using an IDE-like tool like Ozone
    compared to using OpenOCD + **`gdbgui`** and a simpler HW setup. Also,
    Ozone contains more advanced features compared to **`gdbgui`**, but
    their usability depends on the use case and the experience level of
    the user. Finally, the down side of the Segger toolchain is the
    price of the probes and the SW license for Ozone.
