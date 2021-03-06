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
DIR_PKG_SDK=${DIR_BASE_SDK}/pkg
DIR_BIN_SDK=${DIR_PKG_SDK}/bin


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
function do_prepare()
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
        print_info "running SDK build step: ${step}"
        ${DIR_SRC_SANDBOX}/build-sdk.sh ${step} ${DIR_BASE_SDK}
    done

    print_info "OS SDK build complete"
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

        print_info "Building test plan documentation"

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
        TEST_SYSTEM_LOG_DIR=${TEST_LOGS_DIR}/${f}
        mkdir -p ${TEST_SYSTEM_LOG_DIR}

        # derive test or demo project folder from our naming convention
        PROJECT_SRC_DIR=${PROJECT::4}s/${PROJECT}

        # if the project provides a test preparation script, execute this prior
        # to the test run
        TEST_SYSTEM_SETUP=${CURRENT_SCRIPT_DIR}/src/${PROJECT_SRC_DIR}/prepare_test.sh
        if [ -f "${TEST_SYSTEM_SETUP}" ]; then
            ABS_DIR_PGK_SDK=$(realpath ${DIR_PKG_SDK})
            (
                cd ${TEST_SYSTEM_LOG_DIR}
                ${CURRENT_SCRIPT_DIR}/src/${PROJECT_SRC_DIR}/prepare_test.sh ${ABS_DIR_PGK_SDK}
            )
        fi

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

        PYTHON_PARAMS=(
            -B  # do not create *.pyc files
            -m pytest  # execute pytest
            #--------------------------------------------------
            # all parameters below are fed into pytest
            #--------------------------------------------------
            -p no:cacheprovider  # don't create .cache directories
            -v  # increase pytest verbosity (-vv is even more verbose)
            #--tb=short  # control the traceback (long, short, line, native, no)
            #-o log_cli=True  # write logs to console
            #--capture=no  # show printf() from pytest scripts in console
            #--collect-only  # show test, but don't run them
            #--exitfirst  # exit on first test error
            --junitxml=$(realpath ${TEST_LOGS_DIR})/test_results.xml
            #--------------------------------------------------
            # test framework parameters
            #--------------------------------------------------
            #--print_logs  # show log output from device in console
            --target=${BUILD_PLATFORM}
            --system_image=$(realpath ${BUILD_FOLDER}/images/os_image.elf)
            --proxy=$(realpath ${DIR_BIN_SDK}/proxy_app),${QEMU_CONN}
            --log_dir=$(realpath ${TEST_LOGS_DIR})
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

#BUILD_PLATFORM=${BUILD_PLATFORM:-"zynq7000"}
BUILD_PLATFORM=${BUILD_PLATFORM:-"sabre"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"qemu-sabre"}
#BUILD_PLATFORM=${BUILD_PLATFORM:-"rpi3"}


if [[ "${1:-}" == "prepare" ]]; then
    shift
    do_prepare

elif [[ "${1:-}" == "doc" ]]; then
    shift
    build_test_plan_docs


elif [[ "${1:-}" == "run" ]]; then
    shift
    run_tests ${BUILD_PLATFORM} $@


elif [[ "${1:-}" == "qemu" ]]; then
    shift
    QEMU_SYSTEM="build-${BUILD_PLATFORM}-Debug-${1:-demo_hello_world}"
    export BUILD_PLATFORM
    ${DIR_SRC_SANDBOX}/scripts/run_qemu.sh ${QEMU_SYSTEM}/images/os_image.elf


else
    echo "invalid parameter, use"
    echo "   prepare"
    echo "   run [pytest_params_and_args]"
    exit 1

fi
