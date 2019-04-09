#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

# source dir is where this script is located
DIR_SRC=$(dirname $0)

#-------------------------------------------------------------------------------
function run_build()
{
    if [[ -z ${TARGET_DIR:-} ]]; then
        TARGET_DIR=${TARGET}
    fi

    echo ""
    echo "##"
    echo "## building: ${TARGET_DIR}"
    echo "##"

    # build dir will be a subdirectory of the current directory, where this
    # script is invoked in.
    BUILD_DIR=$(pwd)/build-${TARGET_DIR}

    if [[ ! -e ${BUILD_DIR} ]]; then
        # use subshell to configure the build
        (
            mkdir -p ${BUILD_DIR}
            cd ${BUILD_DIR}

            CMAKE_PARAMS=(
                -DCMAKE_TOOLCHAIN_FILE=../${DIR_SRC}/kernel/gcc.cmake
                -DLibLwip=OFF
                -DKernelVerificationBuild=OFF
            )

            cmake ${CMAKE_PARAMS[@]} $@ -G Ninja ../${DIR_SRC}

            # must run cmake multiple times, so config settings propagate properly
            echo "re-run cmake (1/2)"
            cmake .
            echo "re-run cmake (2/2)"
            cmake .
        )
    fi

    # build in subshell
    (
        cd ${BUILD_DIR}
        ninja
    )
}


#-------------------------------------------------------------------------------
function run_build_mode()
{
    BUILD_TARGET=${1:-}
    shift

    BUILD_TYPE=${1:-}
    shift

    TARGET_DIR=${BUILD_TARGET}-${BUILD_TYPE}

    case "${BUILD_TARGET}" in
        #-------------------------------------
        zynq7000)
            CMAKE_TARGET_PARAMS=(
                -DCROSS_COMPILER_PREFIX=arm-linux-gnueabi-
                -DKernelARMPlatform=${BUILD_TARGET}
            )
            ;;
        #-------------------------------------
        spike)
            CMAKE_TARGET_PARAMS=(
                -DCROSS_COMPILER_PREFIX=riscv64-unknown-linux-gnu-
                -DKernelRiscVPlatform=${BUILD_TARGET}
                -DKernelArch=riscv
                -DKernelRiscVSel4Arch=riscv64
            )
            ;;
        #-------------------------------------
        *)
            echo "invalid target: ${BUILD_TARGET}"
            exit 1
    esac

    CMAKE_PARAMS=(
        ${CMAKE_TARGET_PARAMS[@]}
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
        $@
    )
    run_build ${CMAKE_PARAMS[@]}
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
if [[ "${1:-}" == "all" ]]; then

    shift
    run_build_mode zynq7000 Debug -DENABLE_LINT=OFF $@
    #run_build_mode spike Debug -DENABLE_LINT=OFF $@

    run_build_mode zynq7000 Release -DENABLE_LINT=OFF $@
    #run_build_mode spike Release -DENABLE_LINT=OFF $@

elif [[ "${1:-}" == "ci" ]]; then
    shift
    run_build_mode zynq7000 Debug $@
    run_build_mode zynq7000 Release  $@
else
    run_build_mode zynq7000 Debug $@
fi
