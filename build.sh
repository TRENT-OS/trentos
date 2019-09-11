#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

BUILD_SCRIPT_DIR=$(cd `dirname $0` && pwd)

SEOS_SANDBOX_DIR="${BUILD_SCRIPT_DIR}/seos_sandbox"


#-------------------------------------------------------------------------------
function prepare_layout()
{
    # long all folder from src into seso_sandbox, so we can use the sanbox
    # CMake build system and it will find everything from src

    local SRC_DIR="${BUILD_SCRIPT_DIR}/src"
    files=`ls ${SRC_DIR}`
    for file in ${files}; do
        ln -sf ${SRC_DIR}/${file} ${SEOS_SANDBOX_DIR}/projects
    done
}


#-------------------------------------------------------------------------------
function clean_layout()
{
    # long all folder from src into seso_sandbox, so we can use the sanbox
    # CMake build system and it will find everything from src

    local SRC_DIR="${BUILD_SCRIPT_DIR}/src"
    files=`ls ${SRC_DIR}`
    for file in ${files}; do
        rm -f ${SEOS_SANDBOX_DIR}/projects/${file}
    done
}

#-------------------------------------------------------------------------------
function run_astyle()
{
    echo "##"
    echo "## running astyle check ..."
    echo "##"

    # ensure there are not existing astyle files anywhere
    find . -name '*.astyle' -exec rm -v {} \;

    # there should be an astyle script in the root folder, but we also search
    # recursively in all subfolders, as modules can come with their own version
    # of the script
    find . -name 'astyle_check.sh' -printf 'running %p\n' -execdir {} \;
}

#-------------------------------------------------------------------------------
function check_astyle_artifacts()
{
    # *.astyle files are generated when file are not astyle compliant. Check if
    # any such file exists in the workspace and fail in this case.
    local files=$(find . -name '*.astyle')
    if [ ! -z "${files}" ]; then
        exit 1
    fi
}

#-------------------------------------------------------------------------------
function run_build_prepare()
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
}

#-------------------------------------------------------------------------------
function run_build()
{
    run_build_prepare $@

    # build in subshell
    (
        cd ${BUILD_DIR}
        ninja
    )
}

#-------------------------------------------------------------------------------
function run_build_doc()
{
    BUILD_TARGET=${1:-}
    shift

    TARGET_DIR=${BUILD_TARGET}
    run_build_prepare $@

    # build in subshell
    (
        cd ${BUILD_DIR}
        ninja seos_sandbox_doc seos_tests_doc
    )
}

#-------------------------------------------------------------------------------
function run_build_mode()
{
    if [ "$#" -ne 3 ]; then
        echo "ERROR: not enough parameters"
        return 1
    fi

    BUILD_TARGET=${1}
    BUILD_TYPE=${2}
    TEST_NAME=${3}
    shift 3

    TARGET_DIR=${BUILD_TARGET}-${BUILD_TYPE}-${TEST_NAME}

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
        -D${TEST_NAME}=ON
        $@
    )

    run_build ${CMAKE_PARAMS[@]}
}

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

prepare_layout

if [[ "${1:-}" == "doc" ]]; then
    shift
    run_build_doc DOC -DSEOS_SANDBOX_DOC=ON $@

elif [[ "${1:-}" == "all" ]]; then
    shift

    TESTS_MODULES=(
        # HELLO_WORLD
        TEST_SYSLOG
        TEST_CRYPTO_API
        TEST_PROXY_NVM
        TEST_SPIFFS_INTEGRATION
        TEST_PICOTCP_API
        #DEMOS
        KEYSTORE_DEMO_APP
    )

    for test_module in ${TESTS_MODULES[@]}; do
        run_build_mode zynq7000 Debug ${test_module} $@
    done

    run_astyle

elif [[ "${1:-}" == "check_astyle_artifacts" ]]; then
    shift

    # astyle failures do not abort the build normally, but in CI we don't allow
    # them to happen. Thus this step runs after the build step and we allows
    # differentiate build failures from astyle failures easily then.
    check_astyle_artifacts

elif [[ "${1:-}" == "clean" ]]; then
    shift

    /bin/rm -rf build-*
    clean_layout

else

    run_build_mode zynq7000 Debug $@
    run_astyle

fi
