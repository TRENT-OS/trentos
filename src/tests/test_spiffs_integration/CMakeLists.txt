cmake_minimum_required(VERSION 3.7.2)

project(tests_proxy_nvm C)

set(SEOS_LIBS ON CACHE BOOL "string" FORCE)
set(SPIFFS ON CACHE BOOL "string" FORCE)
set(SEOS_LIBS_CRYPTO ON CACHE BOOL "string")

set(DEBUG_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Debug_Config.h"
        CACHE STRING "" FORCE
)
set(MEMORY_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Memory_Config.h"
        CACHE STRING "" FORCE
)
set(LOGS_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Logs_Config.h"
        CACHE STRING "" FORCE
)

# Include cmake helper functions
include(cmake/helpers.cmake)

set(ENABLE_LINT ON
    CACHE BOOL "string")

ImportCamkes()

# COMPONENT TestSpiffsIntegration
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()
DeclareCAmkESComponent(TestSpiffsIntegration
    INCLUDES
        include
    SOURCES
        "components/TestSpiffsIntegration/src/testSpiffsIntegration.c"
    LIBS
        seos_libs
)

#-------------------------------------------------------------------------------
# Component UartDrv
#-------------------------------------------------------------------------------

DeclareCAmkESComponent(UartDrv
    SOURCES
        "components/Uart/src/UartDrv.c"
        "components/Uart/src/qemu_uart.c"
    C_FLAGS
        -Wall -Werror
)

#-------------------------------------------------------------------------------
# Component ChanMux
#-------------------------------------------------------------------------------

DeclareCAmkESComponent(ChanMux
    INCLUDES
        "../configs/include"
    SOURCES
        "components/ChanMux/src/instance/ChanMux_Instance.c"
    C_FLAGS
        -Wall -Werror
        -DSEOS_SOCKET_CONFIG_H_FILE=${SEOS_SOCKET_CONFIG_H_FILE}
        -DMEMORY_CONFIG_H_FILE=${MEMORY_CONFIG_H_FILE}
        -DDEBUG_CONFIG_H_FILE=${DEBUG_CONFIG_H_FILE}
    LIBS
        seos_libs
)

#ROOT SERVER
DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()

set(IMAGE_NAME capdl-loader-image-arm-${KernelARMPlatform})
add_custom_command(
      TARGET rootserver_image
      POST_BUILD
      COMMAND ${CROSS_COMPILER_PREFIX}size ${IMAGE_NAME}  | tee ${IMAGE_NAME}.size
      COMMAND ${CROSS_COMPILER_PREFIX}objdump -dht ${IMAGE_NAME} > ${IMAGE_NAME}.lst
      COMMAND ${CROSS_COMPILER_PREFIX}objdump -dghlsStx ${IMAGE_NAME} > ${IMAGE_NAME}.dump
      WORKING_DIRECTORY "../../images"
      VERBATIM
)

GenerateSimulateScript()

