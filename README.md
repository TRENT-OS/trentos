# seos\_tests

Test applications project.
The aim of this project is to have a codebase that can build various
sel4-\>camkes-\>seos applications performing tests with all the needed
runtime.

## Getting Started

The project builds a seL4 binary image

### Dependencies

* sel4
* muslc
* utils
* sel4muslcsys
* sel4platsupport
* sel4utils
* sel4debug
* sel4allocman
* camkes
* capdl

### Build preparation

Get the seL4 docker build environment from
https://github.com/SEL4PROJ/seL4-CAmkES-L4v-dockerfiles and start it. Use
target "user_sel4" for non-CAmkES buids and "user_camkes" for CAmkES builds.

### Build steps

Doing a out-of-source-folder build is highly recommended, ie. the build folder
should not be in the directory where the sources are checked out. Instead,
check out the sources into a subfolder and start the build from the parent
folder. Then it will create another subfolder parallel to the source folder
with the build artifacts. This leaves the whole source folder unchanged.

    # check out sources into a sub-folder "src"
    git clone --recursive -b master ssh://bitbucket.hensoldt-cyber.systems:7999/hc/seos_tests.git src

    # start build with code in sub-folder "src", will create a new folder
    # "build-<target>" with the binaries
    src/build.sh TEST_NAME

    # start simulation. Terminate QEMU with "Crtl+A X". Note that the build
    # platform is still hard-coded in this script, change if another target
    # platform is used.
    src/run_qemu.sh TEST_NAME


### Build options

#### Enable static code analysis

To build with enabled static code analysis (cppcheck, clang-tidy) the CMake flag
"ENABLE_LINT" has to be set.

    ./build.sh all -D ENABLE_LINT=ON

#### Build all supported tests

To build all supported build, which is Debug and Release at the moment for
the Zynq7000 platform, the parameter "all" can be used

    ./build.sh all
