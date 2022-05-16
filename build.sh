#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019-2022, HENSOLDT Cyber GmbH
#
#-------------------------------------------------------------------------------

BUILD_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"
DIR_SRC_SANDBOX="${BUILD_SCRIPT_DIR}/seos_sandbox"
SDK_OUT_DIR="OS-SDK"
SDK_PKG_OUT_DIR="${SDK_OUT_DIR}/pkg"

# Test settings
DIR_SRC_TA="${BUILD_SCRIPT_DIR}/ta"
WORKSPACE_TEST_FOLDER="workspace_test"


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

    # native systems, require compiling with -DSDK_USE_CAMKES=0
    native_sel4test,src/native/sel4test
    native_sel4bench,src/native/sel4bench
    native_hello_world,src/native/hello_world

    # tests
    test_timeserver,src/tests/test_timeserver
    test_crypto_api,src/tests/test_crypto_api
    test_certserver,src/tests/test_certserver
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
    demo_iot_app_imx6,src/demos/demo_iot_app_imx6
    demo_network_filter,src/demos/demo_network_filter
    demo_raspi_ethernet,src/demos/demo_raspi_ethernet
    demo_tls_api,src/demos/demo_tls_api
    demo_i2c,src/demos/demo_i2c
    demo_tls_server,src/demos/demo_tls_server
)


ALL_PROJECTS_EXCLUDE_zynq7000=(
    demo_iot_app_rpi3
    demo_raspi_ethernet
    demo_iot_app_imx6
)

ALL_PROJECTS_EXCLUDE_rpi3=(
    demo_iot_app
)


#-------------------------------------------------------------------------------
function run_astyle()
{
    echo "##"
    echo "## running astyle check ..."
    echo "##"

    (
        cd ${BUILD_SCRIPT_DIR}

        # Use astyle_check_sdk.sh from seos_sandbox to check astyle issues also
        # in seos_tests and its subfolders / submodules.
        ${DIR_SRC_SANDBOX}/astyle_check_sdk.sh
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

    ${DIR_SRC_SANDBOX}/build-sdk.sh ${BUILD_ACTION} ${BUILD_DIR}
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
    if [[ "${PROJECT_DIR}" =~ ^.*/src/native/(sel4test|sel4bench|hello_world)$ ]]; then
        echo "seL4 native project, disable CAMKES and LINTING"
        PARAMS+=(
            -DSDK_USE_CAMKES=0
        )
    fi

    # special handling for the sel4test project executed on QEMU
    if [[ "${PROJECT_DIR}" =~ ^.*/src/native/sel4test ]] && [[ "${SIMULATION:-false}" = true ]]; then
        echo "sel4test project running in QEMU, disable cache and timer tests"
        PARAMS+=(
            -DSel4testHaveCache=0
            -DSel4testHaveTimer=0
        )
    fi

    if [ ! -d ${SDK_DIR} ]; then
        echo "missing SDK in ${SDK_DIR}"
        return 1
    fi

    # Show the actual command line that is executed to start the system build
    # using the SDK package.
    (
        set -x
        ${SDK_DIR}/build-system.sh ${PARAMS[@]} $@
    )
}


#-------------------------------------------------------------------------------
# Parameters:
#   BUILD_PLATFORM: target platform
#   BUILD_TYPE: Debug/Release
#   PATH_OR_PROJECT: path to project or name from WELL_KNOWN_PROJECTS
function run_sdk_and_system_build()
{
    if [ "$#" -lt 3 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local BUILD_PLATFORM=${1}
    local BUILD_TYPE=${2}
    local PATH_OR_PROJECT="${3}"
    shift 3 # all other params are passed to the project build

    local PROJECT_DIR=""
    if [ -d "${PATH_OR_PROJECT}" ]; then
        PROJECT_DIR="${PATH_OR_PROJECT}"
    else
        for PROJECT in ${WELL_KNOWN_PROJECTS[@]}; do
            local PRJ_NAME=${PROJECT%,*}
            local PRJ_DIR=${PROJECT#*,}
            if [[ "${PATH_OR_PROJECT}" == "${PRJ_NAME}" ]]; then
                if [[ "${PRJ_DIR}" == "-" ]]; then
                    echo "ERROR: no project directory for ${PRJ_NAME}"
                    print_usage_help
                    exit 1
                fi
                PROJECT_DIR="${BUILD_SCRIPT_DIR}/${PRJ_DIR}"
                break;
            fi
        done

        if [ -z "${PROJECT_DIR}" ]; then
            echo "ERROR: unknown project: ${PATH_OR_PROJECT}"
            print_usage_help
            exit 1
        fi

        echo "Project Name:   ${PATH_OR_PROJECT}"
    fi
    echo "Project Folder: ${PROJECT_DIR}"
    if [ "$#" -gt 0 ]; then
        echo "Parameters:     $@"
    fi

    # first build a source-only SDK package from the SDK sources and then use
    # this to build the system. This does not cost much time and ensures we can
    # build a the system with the SDK package. We don't need to build the full
    # package, because no SDK tools or docs are needed to build a system. In
    # case the SDK shall really be used directly, simply comment out the
    # "run_build_sdk" step and pass ${DIR_SRC_SANDBOX} to "run_system_build"
    run_build_sdk collect-sources ${SDK_OUT_DIR}

    local PARAMS=(
        ${SDK_PKG_OUT_DIR}  # ${DIR_SRC_SANDBOX} to use the SDK sources directly
        ${PROJECT_DIR}
        ${BUILD_PLATFORM}
        ${BUILD_TYPE}
    )
    run_system_build ${PARAMS[@]} $@
    run_astyle
}


#-------------------------------------------------------------------------------
function build_all_projects()
{
    local BUILD_ACTION_SDK=$1
    local BUILD_TYPE=$2
    shift 2

    ALL_PLATFORMS=(
      #  # --- ARM ---
      #  am335x
      #  # am335x-boneblack
      #  # am335x-boneblue
      #  # apq8064
      #  # bcm2837
      #      rpi3
      #  # bcm2711
      #      rpi4
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
                ${SDK_PKG_OUT_DIR}  # ${DIR_SRC_SANDBOX} to use SDK sources directly
                ${BUILD_SCRIPT_DIR}/${PRJ_DIR}
                ${BUILD_PLATFORM}
                ${BUILD_TYPE}
            )
            run_system_build ${PARAMS[@]} $@

        done
    done

    run_astyle
}


#-------------------------------------------------------------------------------
function do_test_prepare()
{
    # remove folder if it exists already. This should not happen in CI when we
    # have a clean workspace, but it's convenient for local builds
    if [ -d ${WORKSPACE_TEST_FOLDER} ]; then
        rm -rf ${WORKSPACE_TEST_FOLDER}
    fi
    mkdir ${WORKSPACE_TEST_FOLDER}

    # if we have a SDK package, these steps are no longer required, because
    # they have been executed when the packages was created and released. Since
    # we use seos_sandbox, we have to build the SDK package here and also give
    # it some testing
    for step in collect-sources run-unit-tests build-tools; do
        echo "##"
        echo "## running SDK build step: ${step}"
        echo "##"
        ${DIR_SRC_SANDBOX}/build-sdk.sh ${step} "${WORKSPACE_TEST_FOLDER}/${SDK_OUT_DIR}"
    done

    echo "##"
    echo "### OS SDK build complete"
    echo "##"
}


#-------------------------------------------------------------------------------
function build_test_plan_docs()
{
    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    (
        cd ${WORKSPACE_TEST_FOLDER}
        echo "##"
        echo "## Building test plan documentation"
        echo "##"
        mkdir -p doc
        cd doc
        export PYTHONPATH="${DIR_SRC_TA}/common:${DIR_SRC_TA}/tests"
        # Sometimes there can be odd errors, use this to analyze them in
        # detail
        #   for f in ${DIR_SRC_TA}/tests/*.py; do
        #       echo ${f}
        #       pydoc3 -w ${f}
        #   done
        python3 -B -m pydoc -w ${DIR_SRC_TA}/tests/*.py
    )
}


#-------------------------------------------------------------------------------
# Params: BUILD_PLATFORM [QEMU_CONN] [pytest params ...]
function run_tests()
{
    local BUILD_PLATFORM=$1
    shift

    # the workspace holds the SDK+tools, temporary files and partition images
    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    # There must be an SDK package in the workspace. We don't check details
    # about the the content here, because it depends on a specific test what is
    # needed, thus tests must take care of such details.
    local DIR_PKG_SDK=${WORKSPACE_TEST_FOLDER}/${SDK_PKG_OUT_DIR}
    if [ ! -d ${DIR_PKG_SDK} ]; then
        echo "ERROR: missing SDK package"
        exit 1
    fi

    # proxy connection defaults to TCP. We always pass the proxy params, even
    # if the actual system under test does not use the proxy.
    local QEMU_CONN="${1}"
    if [[ ${QEMU_CONN} != "PTY" && ${QEMU_CONN} != "TCP" ]]; then
        QEMU_CONN="TCP"
        echo "QEMU connection: ${QEMU_CONN} (using default)"
    else
        echo "QEMU connection: ${QEMU_CONN}"
        shift # consume the parameter
    fi

    # process command line parameters. Note that QEMU_CONN above we may have
    # consumed one parameter already.
    local TEST_SCRIPTS=()
    local TEST_PARAMS=()
    for param in $@; do
        # everything that starts with a dash is considered a parameter,
        # otherwise it's considered a script name.
        if [[ ${param} =~ -.*  ]]; then
            TEST_PARAMS+=(${param})
        else
            TEST_SCRIPTS+=(${param})
        fi
    done

    # folder where the logs are created. Partition images can be placed here if
    # they should be archived with the log
    if [ -z "${TEST_LOGS_DIR:-}" ]; then
        local TEST_LOGS_DIR=test-logs-$(date +%Y%m%d-%H%M%S)
        echo "TEST_LOGS_DIR not set, using ${TEST_LOGS_DIR}"
    else
        echo "TEST_LOGS_DIR is ${TEST_LOGS_DIR}"
    fi

    # usually the test script name matches the system name, but there are some
    # special cases
    declare -A TEST_SCRIPT_MAPPING=(
        [test_demo_hello_world]=demo_hello_world
        [test_demo_iot_app]=demo_iot_app
        [test_demo_tls_api]=demo_tls_api
        [test_native_sel4test]=sel4test
        [test_native_sel4bench]=sel4bench
        [test_native_hello_world]=hello_world
    )

    for TEST_SCRIPT in ${TEST_SCRIPTS}; do
        f=$(basename ${TEST_SCRIPT})
        f=${f%.*}
        PROJECT=${TEST_SCRIPT_MAPPING[${f}]:-}
        if [ -z "${PROJECT}" ]; then
            PROJECT=${f}
        fi

        # create the folder to run the test in and collect all output and test
        # setup files in
        local TEST_SYSTEM_LOG_DIR=${TEST_LOGS_DIR}/${f}
        mkdir -p ${TEST_SYSTEM_LOG_DIR}

        # derive test or demo project folder from our naming convention
        local PROJECT_SRC_DIR=${PROJECT::4}s/${PROJECT}

        # if the project provides a test preparation script, execute this prior
        # to the test run
        local ABS_TEST_SYSTEM_SETUP=${BUILD_SCRIPT_DIR}/src/${PROJECT_SRC_DIR}/prepare_test.sh
        if [ -f "${ABS_TEST_SYSTEM_SETUP}" ]; then
            (
                ABS_DIR_PGK_SDK=$(realpath ${DIR_PKG_SDK})
                cd ${TEST_SYSTEM_LOG_DIR}
                ${ABS_TEST_SYSTEM_SETUP} ${ABS_DIR_PGK_SDK}
            )
        fi

        local BUILD_FOLDER="build-${BUILD_PLATFORM}-Debug-${PROJECT}"
        echo "##"
        echo "## running test for system: ${PROJECT}"
        echo "##"

        PYTHON_PARAMS=(
            -B  # do not create *.pyc files
            -m pytest  # execute pytest
            #--------------------------------------------------
            # all parameters below are fed into pytest
            #--------------------------------------------------
            -p no:cacheprovider  # don't create .cache directories
            -v  # increase pytest verbosity (-vv is even more verbose)
            #-o log_cli=True  # write logs to console
            #--capture=no  # show printf() from pytest scripts in console
            #--tb=short  # control the traceback (long, short, line, native, no)
            #--collect-only  # show test, but don't run them
            #--exitfirst  # exit on first test error
            --junitxml=$(realpath ${TEST_LOGS_DIR})/test_results.xml
            #--------------------------------------------------
            # test framework parameters
            #--------------------------------------------------
            #--print_logs  # show log output from device in console
            --target=${BUILD_PLATFORM}
            --system_image=$(realpath ${BUILD_FOLDER}/images/os_image.elf)
            --proxy=$(realpath ${DIR_PKG_SDK}/bin/proxy_app),${QEMU_CONN}
            --log_dir=$(realpath ${TEST_LOGS_DIR})
            # --sd_card=536870912  # 512 MiB
            # ToDo: The resources are taken from the SDK package and not from
            #       the current sandbox that might be used for development. This
            #       leads to the inconvenient effect that resources changes are
            #       not accessible for the tests unless do_test_prepare() is run
            #       again.
            --resource_dir=$(realpath ${DIR_PKG_SDK}/resources)
            #--------------------------------------------------
            # Default and platform specific configuration file to be used by
            # pytest-testconfig. Passing a config on the command line will
            # merge with them, and in case of overlap will overwrite the default
            --tc-file=${DIR_SRC_TA}/tests/platform_config/default.ini
            --tc-file=${DIR_SRC_TA}/tests/platform_config/${BUILD_PLATFORM}.ini
            #--------------------------------------------------
            ${TEST_PARAMS[@]}
            ${DIR_SRC_TA}/tests/${TEST_SCRIPT}
        )

        (
            cd ${TEST_SYSTEM_LOG_DIR}
            export PYTHONPATH="${DIR_SRC_TA}/common:${DIR_SRC_TA}/tests"
            set -x
            python3 ${PYTHON_PARAMS[@]}
        )

    done

    echo "test run complete"
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------



#-------------------------------------------------------------------------------
function print_usage_help()
{
    echo ""
    echo "Usage: build.sh <target> ..."
    echo ""
    echo " Options for <target> are:"
    echo "   sdk [action, defaults to 'all']   (SDK package build)"
    echo "   all                               (SDK package and all known projects)"
    echo "   all-projects                      (just the projects)"
    echo "   clean                             (delete all build output folders)"
    echo "   test-prepare                      (prepare test workspace)"
    echo "   test-doc                          (create test docs)"
    echo "   test-run [params]                 (run test)"
    echo "   <folder>                          (folder with a project)"
    echo "   <project name>                    (well known project name)"
    echo ""
}

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

# default settings
BUILD_TYPE=${BUILD_TYPE:-"Debug"}
#BUILD_TYPE=${BUILD_TYPE:-"Release"}
#BUILD_TYPE=${BUILD_TYPE:-"RelWithDebInfo"}
#BUILD_TYPE=${BUILD_TYPE:-"MinSizeRel"}

BUILD_PLATFORM=${BUILD_PLATFORM:-"zynq7000"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"sabre"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-sabre"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"nitrogen6sx"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"rpi3"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"rpi4"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"spike32"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"spike64"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-arm-virt"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-arm-virt-a15"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-arm-virt-a53"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-arm-virt-a57"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-arm-virt-a72"}


case "${1:-}" in

    "help"|"")
        print_usage_help
        exit 1
        ;;

    "sdk")
        shift
        BUILD_ACTION_SDK=${1:-all}
        if [ "$#" -gt 1 ]; then
            shift
        fi
        run_build_sdk ${BUILD_ACTION_SDK} ${SDK_OUT_DIR}
        ;;

    "all-projects")
        shift
        # build SDK source-only package and use this to build all projects
        build_all_projects collect-sources ${BUILD_TYPE} $@
        ;;

    "all")
        shift
        # build SDK package with binaries and use this to build all projects
        build_all_projects all ${BUILD_TYPE} $@
        ;;

    "clean")
        rm -rf ${SDK_OUT_DIR}
        rm -rf build-*
        ;;

    "test-prepare")
        shift
        do_test_prepare
        ;;

    "test-doc")
        shift
        build_test_plan_docs
        ;;

    "test-run")
        shift
        run_tests ${BUILD_PLATFORM} $@
        ;;

    *)
        # if we are here, $@ has at least one parameter, which is either a
        # project folder or a name from WELL_KNOWN_PROJECTS
        run_sdk_and_system_build ${BUILD_PLATFORM} ${BUILD_TYPE} $@
        ;;
esac
