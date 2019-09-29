#
# this file is included with add_subdirectory() as external project from the
# sandbox cmake build files. The current folder acts as base directory for the
# project. The actual project cmake file ${BUILD_PROJECT}/CMakeLists.txt is
# included in this file.
#

cmake_minimum_required(VERSION 3.7.2)

find_package(Doxygen)
if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    add_custom_target( seos_tests_doc
        COMMAND
            export DOXYGEN_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
            && ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation for tests with Doxygen"
        VERBATIM )
else(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    message("Doxygen needs to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)


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

SET(SEOS_LIBS_DIR "${CMAKE_SOURCE_DIR}/projects/libs")

macro(seos_set_config_file seos_cfg_file)

    # if seos_cfg_file is a relative path, we assume it to be relative to the
    # cmake file that uses this macro. Unfortunately, this path is not in any
    # SEOS lib's list of include folders, so we must make it an absolute path.
    # This is easy, We can simply prepend it with CMAKE_CURRENT_LIST_DIR, which
    # holds the path of the cmake file that uses this macro.
    # There is one pitcall, macro parameters are immutable, so we need a helper
    # variable here
    set(seos_cfg_file_abs "${seos_cfg_file}")

    if (NOT IS_ABSOLUTE "${seos_cfg_file}")
        set(seos_cfg_file_abs "${CMAKE_CURRENT_LIST_DIR}/${seos_cfg_file}")
    endif()

    if (NOT EXISTS "${seos_cfg_file_abs}")
         message(FATAL_ERROR "SEOS config file not found: ${seos_cfg_file_abs}")
    endif()

    set(DEBUG_CONFIG_H_FILE "${seos_cfg_file_abs}" CACHE STRING "")
    set(MEMORY_CONFIG_H_FILE "${seos_cfg_file_abs}" CACHE STRING "")
    set(LOGS_CONFIG_H_FILE "${seos_cfg_file_abs}" CACHE STRING "")
    set(KEYSTORE_CONFIG_H_FILE "${seos_cfg_file_abs}" ON CACHE STRING "")

endmacro()


macro(seos_use_libs)

    # we maintain a list of SEOS libs here that the sandbox provides. This
    # allows doing a nice sanity check agains what the SEOS projects a trying
    # to use. While this makes it quite impossible to sneak new lib in, there
    # is the additional effort when adding new libs. We may remove this list if
    # it turn out to cause more pain than benefit.
    set(SEOS_KNOWN_LIBS
            SEOS_LIBS
            SEOS_LIBS_LOGGER
            SEOS_NW_STACK
            SEOS_CRYPTO
            SEOS_CRYPTO_IMPL_MBEDTLS
            SEOS_KEYSTORE
            SPIFFS
    )

    foreach(seos_lib ${ARGN})

        if (NOT "${seos_lib}" IN_LIST SEOS_KNOWN_LIBS)
            message(FATAL_ERROR "unknown SEOS library: ${seos_lib}")
        endif()

        set(${seos_lib} ON CACHE BOOL "use ${seos_lib}" FORCE)

    endforeach()

endmacro()


#-------------------------------------------------------------------------------
# CAmkES
#-------------------------------------------------------------------------------

# Currently, all SEOS projects are based on CAmkES, so we include everything
# here for them. If we have project that don't use CAmkES, then this should
# become a macro again provide by the sandbox.

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
