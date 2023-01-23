#
# sel4test wrapper
#
# Copyright (C) 2019-2022, HENSOLDT Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

project(sel4test_wrapper C CXX ASM)

find_package("os-sdk" REQUIRED)

#-------------------------------------------------------------------------------
# configuration
#-------------------------------------------------------------------------------

set(SDK_USE_CAMKES OFF CACHE BOOL "don't use CAmkES" FORCE)
# Set the root task CNode size to be large enough for DTB, timer caps, etc
# Note that this works for the platforms that we have at the moment, and may
# need to be increased in the future
set(KernelRootCNodeSizeBits 13 CACHE STRING "")
os_sdk_set_defaults()


if(SIMULATION)
    set(Sel4testSimulation ON CACHE BOOL "" FORCE)
endif()

if(Sel4testSimulation)
    set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
else()
    set(Sel4testHaveCache ON CACHE BOOL "" FORCE)
endif()


# Set our custom domain schedule
set(KernelDomainSchedule "${CMAKE_CURRENT_LIST_DIR}/src/domain_schedule.c" CACHE INTERNAL "")

# set(LibSel4TestPrintXML ON CACHE BOOL "" FORCE)
#
# set(KernelArmHypervisorSupport ON CACHE BOOL "" FORCE)
#
# set(Sel4testSimulation ON CACHE BOOL "" FORCE)
# set(Sel4testHaveCache OFF CACHE BOOL "" FORCE)
# ApplyCommonSimulationSettings(${KernelSel4Arch})
#
# set(HardwareDebugAPI ON CACHE BOOL "" FORCE)
# set(KernelIOMMU ON CACHE BOOL "" FORCE)
# set(KernelIsMCS ON CACHE BOOL "" FORCE)
# set(KernelNumDomains 16 CACHE STRING "" FORCE)
# set(KernelMaxNumNodes 4 CACHE STRING "" FORCE)
#


if(KernelPlatformQEMUArmVirt)
    if(KernelArmExportPCNTUser AND KernelArmExportPTMRUser)
        set(Sel4testHaveTimer ON CACHE BOOL "" FORCE)
    else()
        set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
    endif()
elseif(
    KernelPlatformZynqmp
    OR KernelPlatformPolarfire
    OR (Sel4testSimulation AND (KernelArchRiscV OR KernelArchARM))
)
    # Frequency settings of the ZynqMP make the ltimer tests problematic
    # Polarfire does not have a complete ltimer implementation
    set(Sel4testHaveTimer OFF CACHE BOOL "" FORCE)
else()
    set(Sel4testHaveTimer ON CACHE BOOL "" FORCE)
endif()

#-------------------------------------------------------------------------------
# build
# Unfortunately, we can't just use add_subdirectory("src" "sel4test") because
# this conflicts with the SDK settings and concepts, so the build fails. Some
# cherry-picking is required.
#-------------------------------------------------------------------------------

message(STATUS "Sel4testSimulation: <${Sel4testSimulation}>")
message(STATUS "Sel4testHaveTimer: <${Sel4testHaveTimer}>")
message(STATUS "Sel4testHaveCache: <${Sel4testHaveCache}>")

# Use the SDK and all seL4 helpers. This automatically includes the seL4 CMmake
# helpers via tools/seL4/cmake-tool/common.cmake, which includes the simulation
# helpers from tools/seL4/cmake-tool/helpers/simulation.cmake
os_sdk_setup()

# We can't use os_sdk_import_sel4(), but have to cherry-pick things.
sel4_import_kernel()
elfloader_import_project()

# Use the actual sel4test project,it also defines a root server.
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
