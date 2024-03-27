#
# sel4bench wrapper
#
# Copyright (C) 2021-2024, HENSOLDT Cyber GmbH
# 
# SPDX-License-Identifier: GPL-2.0-or-later
#

cmake_minimum_required(VERSION 3.7.2)

project(sel4bench C CXX ASM)
find_package(elfloader-tool REQUIRED)
sel4_import_kernel()
elfloader_import_project()
add_subdirectory(src/apps/sel4bench)

# Set the root task CNode size to be large enough for DTB, timer caps, etc
# Note that this works for the platforms that we have at the moment, and may
# need to be increased in the future
set(KernelRootCNodeSizeBits 13 CACHE INTERNAL "")

set(AllowUnstableOverhead ON CACHE BOOL "")

include(simulation)
if(KernelSel4ArchX86_64)
    SetSimulationScriptProperty(MEM_SIZE "3G")
endif()
if(KernelPlatformQEMUArmVirt)
    SetSimulationScriptProperty(MEM_SIZE "2G")
endif()

GenerateSimulateScript()
