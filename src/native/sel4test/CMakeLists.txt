#
# sel4test wrapper
#
# Copyright (C) 2019-2020, Hensoldt Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

#-------------------------------------------------------------------------------
# packages besides the kernel
#-------------------------------------------------------------------------------

find_package(elfloader-tool REQUIRED)


#-------------------------------------------------------------------------------
# configuration
#-------------------------------------------------------------------------------

# Set the root task CNode size to be large enough for DTB, timer caps, etc
# Note that this works for the platforms that we have at the moment, and may
# need to be increased in the future
set(KernelRootCNodeSizeBits 13 CACHE INTERNAL "")

# Set our custom domain schedule
set(KernelDomainSchedule "${CMAKE_CURRENT_LIST_DIR}/src/domain_schedule.c" CACHE INTERNAL "")


#-------------------------------------------------------------------------------
# build
#-------------------------------------------------------------------------------
project(sel4test C CXX ASM)

sel4_import_kernel()
elfloader_import_project()
add_subdirectory(src/apps/sel4test-driver)


#-------------------------------------------------------------------------------
# simulation
#-------------------------------------------------------------------------------

include(simulation)
if(KernelSel4ArchX86_64)
    SetSimulationScriptProperty(MEM_SIZE "3G")
endif()
if(KernelPlatformQEMUArmVirt)
    SetSimulationScriptProperty(MEM_SIZE "2G")
endif()

GenerateSimulateScript()
