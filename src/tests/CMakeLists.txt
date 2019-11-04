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
# SEOS
#-------------------------------------------------------------------------------

# fail safe check here that SEOS_SANDBOX_DIR is set
if (NOT SEOS_SANDBOX_DIR)
    message(FATAL_ERROR "missing SEOS_SANDBOX_DIR")
endif()

if (NOT EXISTS "${SEOS_SANDBOX_DIR}")
    message(FATAL_ERROR "missing SEOS libs at ${SEOS_SANDBOX_DIR}")
endif()

# fail safe check here that SEOS_LIBS_DIR is set
if (NOT SEOS_LIBS_DIR)
    message(FATAL_ERROR "missing SEOS_LIBS_DIR")
endif()

if (NOT EXISTS "${SEOS_LIBS_DIR}")
    message(FATAL_ERROR "missing SEOS libs at ${SEOS_LIBS_DIR}")
endif()


#-------------------------------------------------------------------------------
# use project specific cmake file
#-------------------------------------------------------------------------------

# All SEOS projects are based on CAmkES, so we include things here.
seos_import_camkes()

message("building project: ${BUILD_PROJECT}")
include(${BUILD_PROJECT}/CMakeLists.txt)


#-------------------------------------------------------------------------------
# post processing
#-------------------------------------------------------------------------------

set(IMAGE_NAME "$<TARGET_PROPERTY:rootserver_image,IMAGE_NAME>")
add_custom_command(
      TARGET rootserver_image
      POST_BUILD
      COMMAND ${CROSS_COMPILER_PREFIX}size ${IMAGE_NAME}  | tee ${IMAGE_NAME}.size
      COMMAND ${CROSS_COMPILER_PREFIX}objdump -dht ${IMAGE_NAME} > ${IMAGE_NAME}.lst
      COMMAND ${CROSS_COMPILER_PREFIX}objdump -dghlsStx ${IMAGE_NAME} > ${IMAGE_NAME}.dump
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      VERBATIM
)
