cmake_minimum_required(VERSION 3.7.2)

project(tests_crypto_api C)

# Include cmake helper functions
include(cmake/helpers.cmake)

set(ENABLE_LINT ON
    CACHE BOOL "string")
set(SEOS_LIBS ON
    CACHE BOOL "string" FORCE)
set(SEOS_CRYPTO ON
    CACHE BOOL "string")
set(SEOS_CRYPTO_IMPL_MBEDTLS ON
    CACHE BOOL "string")
set(SEOS_KEYSTORE ON
    CACHE BOOL "string")
set(SPIFFS ON
    CACHE BOOL "string" FORCE)

set(DEBUG_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Debug_Config.h"
        CACHE STRING "" FORCE
)
set(MEMORY_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Memory_Config.h"
        CACHE STRING "" FORCE
)

find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)
if (DOXYGEN_FOUND)
    # note the option ALL which allows to build the docs together with the application
    add_custom_target( seos_tests_doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile && ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/seos_tests_doc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else(DOXYGEN_FOUND)
    message("Doxygen needs to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)

ImportCamkes()

# COMPONENT TEST_CRYPTO
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()
DeclareCAmkESComponent(TEST_CRYPTO INCLUDES
                            include
                            common
                            SOURCES
                            "components/TEST_CRYPTO/src/testRunner.c"
                            "components/TEST_CRYPTO/src/testSignatureRsa.c"
                            "components/TEST_CRYPTO/src/testKeyStore.c"
                            "components/TEST_CRYPTO/src/testCrypto.c"
                            "common/KeyStoreInit.c"
)
set_property(TARGET CAmkESComponent_TEST_CRYPTO
             APPEND PROPERTY COMPONENT_LIBS
             seos_libs
             seos_crypto
             seos_keystore
)

# COMPONENT CRYPTO
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()
DeclareCAmkESComponent(CRYPTO INCLUDES
                            include
                            common
                            SOURCES
                            "components/CRYPTO/instance/Crypto.c"
                            "../../libs/seos_crypto/src/SeosCryptoRpc.c"
                            "../../libs/seos_keystore/src/SeosKeyStoreRpc.c"
                            "common/KeyStoreInit.c"
)
set_property(TARGET CAmkESComponent_CRYPTO

             APPEND PROPERTY COMPONENT_C_FLAGS
)
set_property(TARGET CAmkESComponent_CRYPTO

             APPEND PROPERTY COMPONENT_LIBS
             seos_libs
             seos_crypto
             seos_keystore
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
