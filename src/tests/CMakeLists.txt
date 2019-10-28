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
# SEOS libs
#-------------------------------------------------------------------------------

# The sandbox sets up everything. We just have a fail safe check here that
# SEOS_LIBS_DIR is set.
if (NOT SEOS_LIBS_DIR)
    message(FATAL_ERROR "missing SEOS_LIBS_DIR")
endif()

if (NOT EXISTS "${SEOS_LIBS_DIR}")
    message(FATAL_ERROR "missing SEOS libs at ${SEOS_LIBS_DIR}")
endif()


#-------------------------------------------------------------------------------
# CAmkES
#-------------------------------------------------------------------------------

# Currently, all SEOS projects are based on CAmkES, so we include everything
# here for them. If we have project that don't use CAmkES, then this should
# become a macro again provided by the sandbox.

set(CAMKES_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools/camkes")
set(CAPDL_DIR "${CMAKE_SOURCE_DIR}/projects/camkes/capdl")

set(PYTHON_CAPDL_PATH "${CAPDL_DIR}/python-capdl-tool")
file(GLOB_RECURSE capdl_python ${PYTHON_CAPDL_PATH}/*.py)
set(python_with_capdl    "${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHON_CAPDL_PATH} python")
set(capdl_linker_tool "${python_with_capdl} ${CAPDL_DIR}/cdl_utils/capdl_linker.py")
set(CAPDL_TOOL_HELPERS "${CAPDL_DIR}/capDL-tool/capDL-tool.cmake")

find_program(TPP_TOOL tpp PATHS "${CAMKES_TOOLS_DIR}/tools")

# everything below is what the ImportCamkes() macro actually does
set(CapDLLoaderMaxObjects 20000 CACHE STRING "" FORCE)
set(KernelRootCNodeSizeBits 17 CACHE STRING "" FORCE)
set(KernelNumDomains 1 CACHE STRING "" FORCE)
include("${CAMKES_TOOLS_DIR}/camkes.cmake")
add_subdirectory("${CAPDL_DIR}/capdl-loader-app" capdl-loader-app)
add_subdirectory("${CAMKES_TOOLS_DIR}/libsel4camkes" libsel4camkes)


#-------------------------------------------------------------------------------
# use project specific cmake file
#-------------------------------------------------------------------------------

message("building project: ${BUILD_PROJECT}")
include(${BUILD_PROJECT}/CMakeLists.txt)


#-------------------------------------------------------------------------------
# post processing
#-------------------------------------------------------------------------------

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
