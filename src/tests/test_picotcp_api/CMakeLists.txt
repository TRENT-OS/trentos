cmake_minimum_required(VERSION 3.7.2)

project(tests_picotcp_api C)

# Include cmake helper functions
include(cmake/helpers.cmake)

set(ENABLE_LINT ON
    CACHE BOOL "string")

set(SEOS_LIBS ON
    CACHE BOOL "string" FORCE)

set(SEOS_NW_STACK ON
    CACHE BOOL "string")

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

ImportCamkes()

# COMPONENT TEST_PICOTCP
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()



#-------------------------------------------------------------------------------
# Component NwStack
#-------------------------------------------------------------------------------

if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()

DeclareCAmkESComponent(NwStack
    INCLUDES

    SOURCES
        "components/test_nwstack/NwStack/NW1/instance/Nw.c"
    C_FLAGS
        -Wall -Werror   
    LIBS
        seos_libs
        seos_nw_lib
)

#-------------------------------------------------------------------------------
# Component NwStack 2
#-------------------------------------------------------------------------------


if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()


DeclareCAmkESComponent(NwStack_2
    INCLUDES

    SOURCES
        "components/test_nwstack/NwStack/NW2/instance/Nw2.c"
    C_FLAGS
        -Wall -Werror   
    LIBS
        seos_libs
        seos_nw_lib
)


#-------------------------------------------------------------------------------
# Component NwAPP
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(NwApp
    INCLUDES
        "components/test_nwstack/test_APP/include"
    SOURCES
        "components/test_nwstack/test_APP/test_NwApp/src/Client_App.c"
        "components/test_nwstack/test_APP/network-client-lib/seos_nw_api_interface.c"
    C_FLAGS
        -Wall -Werror
        -DCLIENT_CONFIG=1
    LIBS
        seos_libs
)

#-------------------------------------------------------------------------------
# Component NwAPP 2
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(NwApp_2
    INCLUDES
        "components/test_nwstack/test_APP/include"
    SOURCES
        "components/test_nwstack/test_APP/test_NwApp2/src/Server_App.c"
        "components/test_nwstack/test_APP/network-client-lib/seos_nw_api_interface.c"
    C_FLAGS
        -Wall -Werror
        -DSERVER_CONFIG=1
    LIBS
        seos_libs
)

#-------------------------------------------------------------------------------
# Component ChanMux
#-------------------------------------------------------------------------------

DeclareCAmkESComponent(ChanMux
    INCLUDES
        "../configs/include"
    SOURCES
        "components/test_nwstack/ChanMux/src/instance/ChanMux_Instance.c"
    C_FLAGS
        -Wall -Werror
        -DSEOS_SOCKET_CONFIG_H_FILE=${SEOS_SOCKET_CONFIG_H_FILE}
        -DMEMORY_CONFIG_H_FILE=${MEMORY_CONFIG_H_FILE}
        -DDEBUG_CONFIG_H_FILE=${DEBUG_CONFIG_H_FILE}
    LIBS
        seos_libs
)

#-------------------------------------------------------------------------------
# Component Timer
#-------------------------------------------------------------------------------
DeclareCAmkESComponent(Client
    SOURCES
        "components/test_nwstack/Client/src/client.c"
    C_FLAGS
        -Wall -Werror
)
DeclareCAmkESComponent(Timer
    SOURCES
        "components/test_nwstack/Timer/src/timer.c"
    C_FLAGS
        -Wall -Werror
)


#-------------------------------------------------------------------------------
# Component UartDrv
#-------------------------------------------------------------------------------

DeclareCAmkESComponent(UartDrv
    SOURCES
        "components/test_nwstack/Uart/src/UartDrv.c"
        "components/test_nwstack/Uart/src/qemu_uart.c"
    C_FLAGS
        -Wall -Werror
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

