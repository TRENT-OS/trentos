#
# seL4 native hello world application
#
# Copyright (C) 2019-2024, HENSOLDT Cyber GmbH
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.7.2)

#-------------------------------------------------------------------------------
# use the SDK
find_package("os-sdk" REQUIRED)
# SDK configuration and setup
set(SDK_USE_CAMKES OFF CACHE BOOL "don't use CAmkES" FORCE)
set(KernelRootCNodeSizeBits 13 CACHE STRING "")
os_sdk_set_defaults()
os_sdk_setup()
os_sdk_import_sel4()


#-------------------------------------------------------------------------------
# this is the actual "hello world" application

project(hello_world_app C)

add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL
    main.c
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -g
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        sel4
        muslc
        sel4muslcsys
)


#-------------------------------------------------------------------------------
# define the "hello_world_app" as the root server, ie. the task the seL4 kernel
# will start
os_sdk_create_system("hello_world_app")
