#
# this file is included with add_subdirectory() as external project from the
# sandbox cmake build files. The current folder acts as base directory for the
# project. The actual project cmake file ${BUILD_PROJECT}/CMakeLists.txt is
# included in this file.
#

cmake_minimum_required(VERSION 3.7.2)

#-------------------------------------------------------------------------------
# Lint
#-------------------------------------------------------------------------------

# we enable linting for all projects by default
set(ENABLE_LINT ON CACHE BOOL "enable linting")

if (ENABLE_LINT)
    set(CMAKE_C_CPPCHECK "cppcheck;--enable=warning;--inline-suppr")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")
endif()

#-------------------------------------------------------------------------------
# use project specific cmake file
#-------------------------------------------------------------------------------

if (NOT SEOS_SYSTEM)
    message(FATAL_ERROR "please set SEOS_SYSTEM")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${SEOS_SYSTEM}")
    message(FATAL_ERROR "project not found: ${SEOS_SYSTEM}")
endif()

message("building SEOS system: ${SEOS_SYSTEM}")

# All SEOS projects are based on CAmkES, so we include things here.
seos_import_camkes()

get_filename_component(SEOS_SYSTEM_NAME "${SEOS_SYSTEM}" NAME)
add_subdirectory(${SEOS_SYSTEM} ${SEOS_SYSTEM_NAME})


#-------------------------------------------------------------------------------
# post processing
#-------------------------------------------------------------------------------

set(IMAGE_NAME "$<TARGET_PROPERTY:rootserver_image,IMAGE_NAME>")
add_custom_target(
    seos_image_dump ALL
    DEPENDS ${CMAKE_BINARY_DIR}/${IMAGE_NAME}
    COMMAND ${CROSS_COMPILER_PREFIX}size ${IMAGE_NAME} | tee ${IMAGE_NAME}.size
    COMMAND ${CROSS_COMPILER_PREFIX}objdump -dht ${IMAGE_NAME} > ${IMAGE_NAME}.lst
    COMMAND ${CROSS_COMPILER_PREFIX}objdump -dghlsStx ${IMAGE_NAME} > ${IMAGE_NAME}.dump
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
)
