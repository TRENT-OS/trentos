#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

CURRENT_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"

#-------------------------------------------------------------------------------
# our workspace name
WORKSPACE_TEST_FOLDER=workspace_test


#-------------------------------------------------------------------------------
# sandbox is in a subfolder of the current script dir
DIR_SRC_SANDBOX=${CURRENT_SCRIPT_DIR}/seos_sandbox
# SDK is created in the workspace
DIR_BASE_SDK=${WORKSPACE_TEST_FOLDER}/OS-SDK
DIR_BIN_SDK=${DIR_BASE_SDK}/pkg/bin


#-------------------------------------------------------------------------------
# Test Automation
DIR_SRC_TA=${CURRENT_SCRIPT_DIR}/ta


#-------------------------------------------------------------------------------
# Keystore Provisioning Demo
DIR_SRC_KPD=${CURRENT_SCRIPT_DIR}/src/tests/demo_preprovisioned_keystore


#-------------------------------------------------------------------------------
function print_info()
{
    local INFO=$1

    echo -e "\n\n############ ${INFO}\n"
}


#-------------------------------------------------------------------------------
function build_os_sdk()
{
    # remove folder if it exists already. This should not happen in CI when we
    # have a clean workspace, but it's convenient for local builds
    if [ -d ${WORKSPACE_TEST_FOLDER} ]; then
        rm -rf ${WORKSPACE_TEST_FOLDER}
    fi
    mkdir ${WORKSPACE_TEST_FOLDER}

    # if we have a SDK apackage, these stept are no longer required, because
    # they have been executed when the packages was created and released. Since
    # we still use seos_sandbox, we have to build the SDK package here and
    # also give it some testing
    ${DIR_SRC_SANDBOX}/build-sdk.sh build-bin ${DIR_BASE_SDK}
    ${DIR_SRC_SANDBOX}/build-sdk.sh unit-tests ${DIR_BASE_SDK}

    print_info "OS SDK build complete"
}


#-------------------------------------------------------------------------------
function prepare_test()
{
    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    (
        cd ${WORKSPACE_TEST_FOLDER}

        print_info "Building test plan documentation"
        mkdir -p doc
        # run build in subshell
        (
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
        #mv ${DIR_SRC_TA}/doc .
    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
# Params: BUILD_PLATFORM [QEMU_CONN] [pytest params ...]
function run_tests()
{
    local BUILD_PLATFORM=$1
    shift

    # the workspace holds the SDK+tools, temporary files and partition images,
    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
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
        shift
    fi

    # process command line parameters. Note that QEMU_CONN above we may have
    # consumed one parameter already.
    local TEST_SCRIPTS=()
    local TEST_PARAMS=()
    for param in $@; do
        # everything that starts with a dash is considered a parameter,
        # oherwise it's considered a script name.
        if [[ ${param} =~ -.*  ]]; then
            TEST_PARAMS+=(${param})
        else
            TEST_SCRIPTS+=(${param})
        fi
    done

    # folder where the logs are created. Partition images can be placed here if
    # they should be archived wit the log
    if [ -z "${TEST_LOGS_DIR:-}" ]; then
        local TEST_LOGS_DIR=test-logs-$(date +%Y%m%d-%H%M%S)
        echo "TEST_LOGS_DIR not set, using ${TEST_LOGS_DIR}"
    else
        echo "TEST_LOGS_DIR is ${TEST_LOGS_DIR}"
    fi


    # unfortunately, the system image name contains the architecture in the
    # name also, so we need this table to derive it. It would be much easier
    # if the build process creates the image with a generic name.
    declare -A SEL4_ARCH_MAPPING=(
        [imx6]=arm
        [migv]=riscv
        [rpi3]=arm
        [spike]=arm
        [zynq7000]=arm
    )
    SEL4_ARCH=${SEL4_ARCH_MAPPING[${BUILD_PLATFORM}]:-}
    if [ -z "${SEL4_ARCH}" ]; then
        echo "ERROR: could not determine architecture for platform ${BUILD_PLATFORM}"
        exit 1
    fi
    SYSTEM_IMG="images/capdl-loader-image-${SEL4_ARCH}-${BUILD_PLATFORM}"

    # usually the test script name matched the system name, but there are some
    # special cases
    declare -A IMG_MAPPING=(
        [test_demo_hello_world]=demo_hello_world
        [test_demo_iot_app]=demo_iot_app
    )

    for TEST_SCRIPT in ${TEST_SCRIPTS}; do
        f=$(basename ${TEST_SCRIPT})
        f=${f%.*}
        PROJECT=${IMG_MAPPING[${f}]:-}
        if [ -z "${PROJECT}" ]; then
            PROJECT=${f}
        fi

        # create the folder to run the test in and collect all output and test
        # setup files in
        TEST_SYSTEM_LOG_DIR=${TEST_LOGS_DIR}/${f}
        mkdir -p ${TEST_SYSTEM_LOG_DIR}

        # derive test or demo project folder from our naming convention
        PROJECT_SRC_DIR=${PROJECT::4}s/${PROJECT}
        local BUILD_FOLDER="build-${BUILD_PLATFORM}-Debug-${PROJECT}"

        # ToDo: if ${PROJECT} does keystore test ...
        #
        # (
        #     print_info "Prepare KeyStore image"
        #     ABS_DIR_BIN_SDK=$(realpath ${DIR_BIN_SDK})
        #     cd ${WORKSPACE_TEST_FOLDER}
        #
        #     # Create a fresh keystore image for each test run. The keystore
        #     # provisioning tool currently works by feeding a XML file into
        #     # a python script that will create command line arguments for the
        #     # actual tool. The tool creates a file "nvm_06", which we rename
        #     # into the desired image file name then.
        #
        #     python3 \
        #         -B \
        #         ${ABS_DIR_BIN_SDK}/xmlParser.py \
        #         ${DIR_SRC_KPD}/preprovisionedKeys.xml \
        #         ${ABS_DIR_BIN_SDK}/kpt
        #    mv nvm_06 preProvisionedKeyStoreImg
        # )

        print_info "running test for system: ${PROJECT}"

        PYTEST_PARAMS=(
            -v
            # --capture=no   # show printf() from python scripts in console
            --target=${BUILD_PLATFORM}
            --system_image=$(realpath ${BUILD_FOLDER}/${SYSTEM_IMG})
            --proxy=$(realpath ${DIR_BIN_SDK}/proxy_app),${QEMU_CONN}
            --log_dir=$(realpath ${TEST_LOGS_DIR})
            --junitxml=$(realpath ${TEST_LOGS_DIR})/test_results.xml
        )

        (
            cd ${TEST_SYSTEM_LOG_DIR}
            export PYTHONPATH="${DIR_SRC_TA}/common:${DIR_SRC_TA}/tests"

            set -x
            # run pytest but prevent it from creating any pycache folder
            python3 -B -m pytest -p no:cacheprovider ${PYTEST_PARAMS[@]} \
                        ${TEST_PARAMS[@]} ${DIR_SRC_TA}/tests/${TEST_SCRIPT}
        )

    done

    echo "test run complete"
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

BUILD_PLATFORM=${BUILD_PLATFORM:-"zynq7000"}


if [[ "${1:-}" == "build" ]]; then
    shift

    build_os_sdk


elif [[ "${1:-}" == "prepare" ]]; then
    shift

    # ToDo: the step "build_test_tools" is executed here to keep backwards
    #       compatibility. It will be removed once all script have been updated
    #       to invoke the build step above.
    build_os_sdk
    prepare_test


elif [[ "${1:-}" == "run" ]]; then
    shift
    run_tests ${BUILD_PLATFORM} $@


else
    echo "invalid parameter, use"
    echo "   build"
    echo "   prepare"
    echo "   run [pytest_params_and_args]"
    exit 1

fi
