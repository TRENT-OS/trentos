#
# sel4test wrapper
#
# Copyright (C) 2019-2022, HENSOLDT Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

project(sel4test_wrapper C CXX ASM)

#-------------------------------------------------------------------------------
# process environment settings
#-------------------------------------------------------------------------------

if(SIMULATION)
    set(Sel4testSimulation ON CACHE BOOL "" FORCE)
endif()


#-------------------------------------------------------------------------------
# SDK and kernel configuration
#-------------------------------------------------------------------------------

find_package("os-sdk" REQUIRED)
set(SDK_USE_CAMKES OFF CACHE BOOL "don't use CAmkES" FORCE)

# Set the root task CNode size to be large enough for DTB, timer caps, etc
# Note that this works for the platforms that we have at the moment, and may
# need to be increased in the future
set(KernelRootCNodeSizeBits 13 CACHE STRING "")

# Set our custom domain schedule
set(KernelDomainSchedule "${CMAKE_CURRENT_LIST_DIR}/src/domain_schedule.c" CACHE INTERNAL "")

# set(HardwareDebugAPI ON CACHE BOOL "" FORCE)
# set(KernelIOMMU ON CACHE BOOL "" FORCE)
# set(KernelIsMCS ON CACHE BOOL "" FORCE)
# set(KernelNumDomains 16 CACHE STRING "" FORCE)
# set(KernelMaxNumNodes 4 CACHE STRING "" FORCE)
# set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)

# set SDK defaults, these are not supposed to overwrite anything that was
# already configured above
os_sdk_set_defaults()

# Use the SDK and all seL4 helpers. This automatically includes the seL4 CMmake
# helpers via tools/seL4/cmake-tool/common.cmake, which includes the simulation
# helpers from tools/seL4/cmake-tool/helpers/simulation.cmake
os_sdk_setup()

# We can't use os_sdk_import_sel4(), but have to cherry-pick things.
sel4_import_kernel()
elfloader_import_project()


#-------------------------------------------------------------------------------
# now the seL4 platform configuration is set up, so we can it it for sel4test
# specific config settings.
#-------------------------------------------------------------------------------

# set(LibSel4TestPrintXML ON CACHE BOOL "" FORCE)
# ApplyCommonSimulationSettings(${KernelSel4Arch})

# use all tests by default
set(Sel4testHaveCache ON CACHE BOOL "")
set(Sel4testHaveTimer ON CACHE BOOL "")

if(KernelPlatformQEMUArmVirt)
    # QEMU does not simulate a cache, so these tests are likely to fail
    set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
    # There is no timer peripheral, but we can use the generic timer
    if(NOT KernelArmExportPCNTUser AND NOT KernelArmExportPTMRUser)
        set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
    endif()

elseif(KernelPlatformZynqmp)
    if(Sel4testSimulation)
        set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
        set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
    else()
        # Frequency settings of the ZynqMP makes the ltimer tests problematic
        set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
    endif()

elseif(KernelPlatformZynq7000)
    if(Sel4testSimulation)
        # QEMU does not simulate a cache, so these tests are likely to fail
        set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
    endif()

elseif(KernelPlatformSabre)
    if(Sel4testSimulation)
        # QEMU does not simulate a cache, so these tests are likely to fail
        set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
        # QEMU timer is broken, will be fixed in v8.0 eventually
        set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
    endif()

elseif(KernelPlatformPolarfire)
    # Polarfire does not have a complete ltimer implementation
    set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)

elseif (Sel4testSimulation)
    # This must be the second last check that applies for simulations in case no
    # specific platform setting have be set up.
    message(STATUS "using default simulation settings for KernelPlatform '${KernelPlatform}'")
    # QEMU does not simulate a cache, so these tests are likely to fail
    set(Sel4testHaveCache OFF CACHE BOOL "")
    if (KernelArchRiscV OR KernelArchARM)
        set(Sel4testHaveTimer OFF CACHE BOOL "")
    endif()
else()
    # This must be the last statement that applies for test on real hardware
    message(STATUS "using default settings for KernelPlatform '${KernelPlatform}'")
endif()

message(STATUS "Sel4testHaveTimer: <${Sel4testHaveTimer}>")
message(STATUS "Sel4testHaveCache: <${Sel4testHaveCache}>")

# Use the actual sel4test project,it also defines a root server. We can't just
# use add_subdirectory("src" "sel4test") because this conflicts with the SDK
# settings and concepts, so the build fails. Some cherry-picking is required.
add_subdirectory("src/apps/sel4test-driver" "sel4test")

# Adapting the simulation settings and create the scripts to start the
# simulation.
if(KernelSel4ArchX86_64)
    SetSimulationScriptProperty(MEM_SIZE "3G")
elseif(KernelPlatformQEMUArmVirt)
    SetSimulationScriptProperty(MEM_SIZE "2G")
endif()
GenerateSimulateScript()

os_sdk_postprocess_targets()
