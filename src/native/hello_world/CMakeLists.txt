

cmake_minimum_required(VERSION 3.7.2)

set(KernelRootCNodeSizeBits 13 CACHE INTERNAL "")


#-------------------------------------------------------------------------------
# seL4 build system

find_package(seL4 REQUIRED)
find_package(elfloader-tool REQUIRED)
find_package(musllibc REQUIRED)
find_package(util_libs REQUIRED)
find_package(seL4_libs REQUIRED)

sel4_import_kernel()
elfloader_import_project()


# calling musllibc_setup_build_environment_with_sel4runtime() below requires a
# project to be defined
project(sel4-native-system C CXX ASM)

# This sets up environment build flags and imports musllibc and runtime lib.
musllibc_setup_build_environment_with_sel4runtime()

sel4_import_libsel4()
util_libs_import_libraries()
sel4_libs_import_libraries()


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
# define "hello world" applocation as the root server, ie. the task the seL4
# kernel will start

DeclareRootserver(hello_world_app)


#-------------------------------------------------------------------------------
# generate simulation scripts

GenerateSimulateScript()

