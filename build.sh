#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

BUILD_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"
SDK_SRC_DIR="${BUILD_SCRIPT_DIR}/seos_sandbox"
SDK_OUT_DIR="OS-SDK"

# This list is used for the targets "all-projects" and "all". The order within
# the list starts with the most simple system to build, then moves on to more
# complex test systems and finally has the demos. Rationale is, that if the
# simpler builds fail, there is no point in building more complex things,
# because we likely run into the same problems again. And the building the
# demos does not make any sense if we can't even build the tests.
WELL_KNOWN_PROJECTS=(
    # the entry format is <test name>,<folder>. We don't make the fixed
    # assumption that the folder name matches the test name or that there is a
    # generic subfolder where all tests are. This assumption holds today, but
    # there is a plan to drop using "src/tests" if the whole test system is a
    # GIT submodule and we don't have any local sources.

    demo_hello_world,src/demos/demo_hello_world

    # native systems, require compiling with -DSDK_USE_CAMKES=0 -DENABLE_LINT=0
    native_sel4test,src/native/sel4test
    native_hello_world,src/native/hello_world

    # tests
    test_timeserver,src/tests/test_timeserver
    test_crypto_api,src/tests/test_crypto_api
    test_certparser,src/tests/test_certparser
    test_cryptoserver,src/tests/test_cryptoserver
    test_entropysource,src/tests/test_entropysource
    test_uart,src/tests/test_uart
    test_chanmux,src/tests/test_chanmux
    test_proxy_nvm,src/tests/test_proxy_nvm
    test_filesystem,src/tests/test_filesystem
    test_logserver,src/tests/test_logserver
    test_config_server,src/tests/test_config_server
    test_keystore,src/tests/test_keystore
    test_network_api,src/tests/test_network_api
    test_storage_interface,src/tests/test_storage_interface
    test_secure_update,src/tests/test_secure_update
    test_tls_api,src/tests/test_tls_api
    test_tlsserver,src/tests/test_tlsserver

    # demos
    demo_iot_app,src/demos/demo_iot_app
    demo_iot_app_rpi3,src/demos/demo_iot_app_rpi3
    demo_raspi_ethernet,src/demos/demo_raspi_ethernet
    demo_tls_api,src/demos/demo_tls_api
)


ALL_PROJECTS_EXCLUDE_zynq7000=(
    demo_iot_app_rpi3
    demo_raspi_ethernet
)

ALL_PROJECTS_EXCLUDE_rpi3=(
    demo_iot_app
)


#-------------------------------------------------------------------------------
function map_project()
{
    local VAR_PRJ_DIR=${1}
    local TEST_SYSTEM=${2:-}

    for PROJECT in ${WELL_KNOWN_PROJECTS[@]}; do
        local PRJ_NAME=${PROJECT%,*}
        local PRJ_DIR=${PROJECT#*,}

        if [[ "${TEST_SYSTEM}" == "${PRJ_NAME}" ]]; then

            if [[ "${PRJ_DIR}" == "-" ]]; then
                echo "ERROR: no project directory for ${PRJ_NAME}"
                exit 1
            fi

            eval "${VAR_PRJ_DIR}=${BUILD_SCRIPT_DIR}/${PRJ_DIR}"
            return 0
        fi

    done

    return 1
}


#-------------------------------------------------------------------------------
function run_astyle()
{
    echo "##"
    echo "## running astyle check ..."
    echo "##"

    (
        cd ${BUILD_SCRIPT_DIR}

        # ensure there are no existing astyle files anywhere
        find . -name '*.astyle' -exec rm -v {} \;

        # there should be an astyle script in the root folder, but we also
        # search recursively in all subfolders, as modules can come with their
        # own version of the script
        find . -name 'astyle_check.sh' -printf 'running %p\n' -execdir {} \;

        ${SDK_SRC_DIR}/astyle_check_sdk.sh
    )
}


#-------------------------------------------------------------------------------
function run_build_sdk()
{
    local BUILD_ACTION=$1
    local BUILD_DIR=$2

    echo ""
    echo "##"
    echo "## building SDK Package (${BUILD_ACTION}) in ${BUILD_DIR}"
    echo "##"

    ${SDK_SRC_DIR}/build-sdk.sh ${BUILD_ACTION} ${BUILD_DIR}
}


#-------------------------------------------------------------------------------
function run_system_build()
{
    if [ "$#" -lt 4 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local SDK_DIR=${1}
    local PROJECT_DIR=${2}
    local BUILD_PLATFORM=${3}
    local BUILD_TYPE=${4}
    shift 4

    local PROJECT_NAME=$(basename ${PROJECT_DIR})
    # build output will be generated in this folder
    local BUILD_DIR=build-${BUILD_PLATFORM}-${BUILD_TYPE}-${PROJECT_NAME}

    local PARAMS=(
        # build-system.sh expect at least these parameters
        ${PROJECT_DIR}
        ${BUILD_PLATFORM}
        ${BUILD_DIR}
        #------------------------------------------------
        # every parameter below is passed to CMake
        -D CMAKE_BUILD_TYPE=${BUILD_TYPE}
    )

    # special handling for the seL4 native projects
    if [[ "${PROJECT_DIR}" =~ ^.*/src/native/(sel4test|hello_world)$ ]]; then
        echo "seL4 native project, disable CAMKES and LINTING"
        PARAMS+=(
            -DSDK_USE_CAMKES=0
            -DENABLE_LINT=0
        )
    fi

    if [ ! -d ${SDK_DIR} ]; then
        echo "missing SDK in ${SDK_DIR}"
        return 1
    fi

    ${SDK_DIR}/build-system.sh ${PARAMS[@]} $@
}


#-------------------------------------------------------------------------------
function run_sdk_and_system_build()
{
    if [ "$#" -lt 3 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local PROJECT_DIR=${1}
    local BUILD_PLATFORM=${2}
    local BUILD_TYPE=${3}
    shift 3

    # first build a source-only SDK package from the SDK sources and then use
    # this to build the system. This does not cost much time and ensures we can
    # build a the system with the SDK package. We don't need to build the full
    # package, because no SDK tools or docs are needed to build a system. In
    # case the SDK shall really be used directly, simply comment out the
    # "run_build_sdk" step and pass ${SDK_SRC_DIR}
    run_build_sdk collect-sources ${SDK_OUT_DIR}

    local PARAMS=(
        ${SDK_OUT_DIR}/pkg  # ${SDK_SRC_DIR} to use the SDK sources directly
        ${PROJECT_DIR}
        ${BUILD_PLATFORM}
        ${BUILD_TYPE}
    )
    run_system_build ${PARAMS[@]} $@
}


#-------------------------------------------------------------------------------
function build_all_projects()
{
    local BUILD_ACTION_SDK=$1
    shift

    ALL_PLATFORMS=(
      #  # --- ARM ---
      #  am335x
      #  # am335x-boneblack
      #  # am335x-boneblue
      #  # apq8064
      #  # bcm2837
      #      rpi3
      #  # exynos4
      #  # exynos5 # -> exynos5250
      #  #     exynos5250
      #  #     exynos5410
      #  #     exynos5422
      #  # #fvp    !!!build error
      #  # hikey
      #  # imx6 # -> sabre
      #  #     sabre
      #  #     wandq
      #  # #imx7/imx7sabre, but does not compile)
      #  # imx8mq-evk
      #  # imx8mm-evk
      #  # imx31 # should default to kzm, but does not
      #  #     kzm # imx31
      #  # odroidc2
      #  # omap3
      #  # # qemu-arm-virt  !!!build error
      #  # rockpro64
      #  # tk1
      #  # tx1
      #  # tx2
        zynq7000
      #  zynqmp
      #      # ultra96 #zynqmp, but does not compile
      #
      #  # --- RISC-V ---
      #  # ariane
      #  hifive
      #  # spike
      #
      #  # --- x86 ---
      #  # pc99 # does not compile
    )

    run_build_sdk ${BUILD_ACTION_SDK} ${SDK_OUT_DIR}

    # for now, just loop over the list above and abort the whole build on the
    # first error. Ideally we would not abort here, but try to do all builds
    # and then report which failed. Or better, the caller should invoke this
    # build script several times in parallel for each system.
    for PROJECT in ${WELL_KNOWN_PROJECTS[@]}; do
        local PRJ_NAME=${PROJECT%,*}
        local PRJ_DIR=${PROJECT#*,}

        # if there is no project then do nothing
        if [[ "${PRJ_DIR}" == "-" ]]; then
            continue
        fi

        for BUILD_PLATFORM in ${ALL_PLATFORMS[@]}; do

            eval EXCLUDE_LIST=\${ALL_PROJECTS_EXCLUDE_${BUILD_PLATFORM}[@]}
            local skip=0
            for EXCL_PRJ in ${EXCLUDE_LIST[@]}; do
                if [[ ${EXCL_PRJ} == ${PRJ_NAME} ]]; then
                    echo -e "\nSkipping excluded project: ${PRJ_NAME}"
                    skip=1
                    break
                fi
            done

            if [[ ${skip} -ne 0 ]]; then
                break
            fi

            local PARAMS=(
                ${SDK_OUT_DIR}/pkg   # ${SDK_SRC_DIR} to sue SDK sources directly
                ${BUILD_SCRIPT_DIR}/${PRJ_DIR}
                ${BUILD_PLATFORM}
                Debug
            )
            run_system_build ${PARAMS[@]} $@

        done
    done

    run_astyle
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

# default settings
BUILD_TYPE=${BUILD_TYPE:-"Debug"}

#BUILD_PLATFORM=${BUILD_PLATFORM:-"zynq7000"}
BUILD_PLATFORM=${BUILD_PLATFORM:-"sabre"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"rpi3"}



if [[ "${1:-}" == "sdk" ]]; then
    shift
    BUILD_ACTION_SDK=${1:-all}
    run_build_sdk ${BUILD_ACTION_SDK} ${SDK_OUT_DIR}

elif [[ "${1:-}" == "all-projects" ]]; then
    shift
    # build SDK source-only package and use this to build all projects
    build_all_projects collect-sources $@

elif [[ "${1:-}" == "all" ]]; then
    shift
    run_build_doc $@
    # build SDK package with binaries, then use this to build all projects
    build_all_projects all $@

elif [[ "${1:-}" == "clean" ]]; then
    shift
    rm -rf build-*
    rm -rf ${SDK_OUT_DIR}

elif map_project MAPPED_PROJECT_DIR ${1:-}; then
    echo "building ${1:-} from ${MAPPED_PROJECT_DIR} ..."
    shift
    run_sdk_and_system_build ${MAPPED_PROJECT_DIR} ${BUILD_PLATFORM} ${BUILD_TYPE} $@
    run_astyle

elif [ "$#" -ne "0" ]; then
    BUILD_PROJECT_DIR=${1:-}
    shift
    echo "building ${BUILD_PROJECT_DIR} using params: '$@' ..."
    run_sdk_and_system_build ${BUILD_PROJECT_DIR} ${BUILD_PLATFORM} ${BUILD_TYPE} $@
    run_astyle

else
    echo ""
    echo "Usage: build.sh <target> ..."
    echo ""
    echo " Options for <target> are:"
    echo "   sdk [action, defaults to 'all']   (SDK package build)"
    echo "   all                               (SDK package and all known projects)"
    echo "   all-projects                      (just the projects)"
    echo "   clean                             (delete all build output folders)"
    echo "   <project name>                    (well known project name)"
    echo "   <folder>                          (folder with a project)"
    echo ""
    exit 1
fi
