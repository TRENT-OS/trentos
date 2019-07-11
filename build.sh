#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

# source dir is where this script is located
BUILD_SCRIPT_DIR=$(cd `dirname $0` && pwd)
SEOS_SANDBOX_DIR="$BUILD_SCRIPT_DIR/seos_sandbox"
PROJECTS_DIR="$SEOS_SANDBOX_DIR/projects"
SRC_DIR="$BUILD_SCRIPT_DIR/src"

#-------------------------------------------------------------------------------
function prepare_layout()
{
    files=`ls $SRC_DIR`
    for file in $files; do
        ln -sf $SRC_DIR/$file $PROJECTS_DIR
    done
}

#-------------------------------------------------------------------------------
function run_astyle()
{
    #cleanup
    find . -name '*.astyle' -exec rm {} \;
    # search recursively in all subfolders, they might be git submodules that come with their own astyle_check.sh file
    files=`find . -name 'astyle_check.sh'`
    for file in $files; do
        ($file)
    done
}

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
                -DCMAKE_TOOLCHAIN_FILE=${SEOS_SANDBOX_DIR}/kernel/gcc.cmake
                -DLibLwip=OFF
                -DKernelVerificationBuild=OFF
            )

            cmake ${CMAKE_PARAMS[@]} $@ -G Ninja ${SEOS_SANDBOX_DIR}

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

    TEST_NAME=${1:-}
    shift

    TARGET_DIR=${BUILD_TARGET}-${BUILD_TYPE}-${TEST_NAME}

    case "${BUILD_TARGET}" in
        #-------------------------------------
        zynq7000)
            CMAKE_TARGET_PARAMS=(
                -DCROSS_COMPILER_PREFIX=arm-linux-gnueabi-
                -DKernelARMPlatform=${BUILD_TARGET}
                -D${TEST_NAME}=ON
            )
            ;;
        #-------------------------------------
        spike)
            CMAKE_TARGET_PARAMS=(
                -DCROSS_COMPILER_PREFIX=riscv64-unknown-linux-gnu-
                -DKernelRiscVPlatform=${BUILD_TARGET}
                -DKernelArch=riscv
                -DKernelRiscVSel4Arch=riscv64
                -D${TEST_NAME}=ON
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
prepare_layout

if [[ "${1:-}" == "all" ]]; then
    shift
    run_build_mode zynq7000 Debug HELLO_WORLD $@
    run_build_mode zynq7000 Debug TEST_SYSLOG $@
    run_build_mode zynq7000 Debug TEST_CRYPTO_API $@
    run_build_mode zynq7000 Debug TEST_PROXY_NVM $@
    run_build_mode zynq7000 Debug TEST_SPIFFS_INTEGRATION $@
elif [[ "${1:-}" == "clean" ]]; then
    shift
    /bin/rm -rf build-*
else
    run_build_mode zynq7000 Debug $@
fi

RETVAL=$?

if [ $RETVAL == 0 ]; then
    echo
    run_astyle
fi

exit $RETVAL
