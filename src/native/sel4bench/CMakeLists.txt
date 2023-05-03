#
# sel4bench wrapper
#
# Copyright (C) 2021, HENSOLDT Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

project(sel4bench C CXX ASM)

#-------------------------------------------------------------------------------
# use the SDK
find_package("os-sdk" REQUIRED)

# SDK configuration and setup
set(SDK_USE_CAMKES OFF CACHE BOOL "don't use CAmkES" FORCE)
os_sdk_set_defaults()

# Set the root task CNode size to be large enough for DTB, timer caps, etc
# Note that this works for the platforms that we have at the moment, and may
# need to be increased in the future
set(KernelRootCNodeSizeBits 13 CACHE INTERNAL "")

set(AllowUnstableOverhead ON CACHE BOOL "")

os_sdk_setup()

# using os_sdk_import_sel4() gives everything, just include what we need
sel4_import_kernel()
elfloader_import_project()

add_subdirectory(src/apps/sel4bench)


include(simulation)
if(KernelSel4ArchX86_64)
    SetSimulationScriptProperty(MEM_SIZE "3G")
endif()
if(KernelPlatformQEMUArmVirt)
    SetSimulationScriptProperty(MEM_SIZE "2G")
endif()

GenerateSimulateScript()
