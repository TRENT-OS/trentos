cmake_minimum_required(VERSION 3.7.2)

project(tests_crypto_api C)

# Include cmake helper functions
include(cmake/helpers.cmake)

set(ENABLE_LINT ON
    CACHE BOOL "string")
set(SEOS_LIBS ON
    CACHE BOOL "string" FORCE)
set(SEOS_CRYPTO_COMPONENT ON
    CACHE BOOL "string")
set(SEOS_CRYPTO_IMPL_MBEDTLS ON
    CACHE BOOL "string")

set(DEBUG_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Debug_Config.h"
        CACHE STRING "" FORCE
)
set(MEMORY_CONFIG_H_FILE
        "${CMAKE_SOURCE_DIR}/projects/configs/include/Memory_Config.h"
        CACHE STRING "" FORCE
)
ImportCamkes()

# COMPONENT TEST_CRYPTO
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()
DeclareCAmkESComponent(TEST_CRYPTO INCLUDES
                            include
                            SOURCES
                            "components/TEST_CRYPTO/src/test_crypto.c"
                            "components/TEST_CRYPTO/src/testSignatureRsa.c"
)
set_property(TARGET CAmkESComponent_TEST_CRYPTO
             APPEND PROPERTY COMPONENT_LIBS
             seos_libs
             seos_crypto
)

# COMPONENT CRYPTO
if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()
DeclareCAmkESComponent(CRYPTO INCLUDES
                            include
                            SOURCES
                            "components/CRYPTO/instance/Crypto.c"
)
set_property(TARGET CAmkESComponent_CRYPTO

             APPEND PROPERTY COMPONENT_C_FLAGS
)
set_property(TARGET CAmkESComponent_CRYPTO

             APPEND PROPERTY COMPONENT_LIBS
             seos_libs
             seos_crypto
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

