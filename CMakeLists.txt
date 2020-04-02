#
# this file is included with add_subdirectory() as external project from the
# sandbox cmake build files. The current folder acts as base directory for the
# project. The actual project cmake file ${BUILD_PROJECT}/CMakeLists.txt is
# included in this file.
#

cmake_minimum_required(VERSION 3.7.2)


#-------------------------------------------------------------------------------
# use project specific cmake file
#-------------------------------------------------------------------------------

if (NOT OS_SYSTEM)
    message(FATAL_ERROR "please set OS_SYSTEM")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${OS_SYSTEM}")
    message(FATAL_ERROR "project not found: ${OS_SYSTEM}")
endif()

message("building OS system: ${OS_SYSTEM}")

# All OS projects are based on CAmkES, so we include things here.
seos_import_camkes()

get_filename_component(OS_SYSTEM_NAME "${OS_SYSTEM}" NAME)
add_subdirectory(${OS_SYSTEM} ${OS_SYSTEM_NAME})
