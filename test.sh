#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

CURRENT_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"
WORKSPACE_ROOT=$(pwd)


#-------------------------------------------------------------------------------
# our workspace name
WORKSPACE_TEST_FOLDER=workspace_test


#-------------------------------------------------------------------------------
# sandbox is in a subfolder of the current script dir
DIR_SRC_SANDBOX=${CURRENT_SCRIPT_DIR}/seos_sandbox
# SDK is created in the workspace
DIR_BASE_SDK=${WORKSPACE_TEST_FOLDER}/OS-SDK
DIR_BIN_SDK=${DIR_BASE_SDK}/pkg/bin
ABS_DIR_BIN_SDK=${WORKSPACE_ROOT}/${DIR_BIN_SDK}


# Keystore Provisioning Tool usage wrapper
function sdk_kpt()
{
    local CFG_XML=$1
    local IMG_OUT=$2

    # The keystore provisioning tool currently works by feeding a XML file into
    # a python script that will create command line arguments for the actual
    # tool. The tool creates a file "nvm_06", which we rename into the desired
    # image file name then.
    # Since this function can be called from a different working directory, we
    # can't use DIR_BIN_SDK, but need ABS_DIR_BIN_SDK with the absolute path.
    python3 ${ABS_DIR_BIN_SDK}/xmlParser.py ${CFG_XML} ${ABS_DIR_BIN_SDK}/kpt

    mv nvm_06 ${IMG_OUT}
}


#-------------------------------------------------------------------------------
# Test Automation
DIR_SRC_TA=${CURRENT_SCRIPT_DIR}/ta
FOLDER_BUILD_TA=ta


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
            pydoc3 -w ${DIR_SRC_TA}/tests/*.py
        )
        #mv ${DIR_SRC_TA}/doc .
    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
function run_test()
{
    if [ -z "${TEST_RUN_ID:-}" ]; then
        local TEST_RUN_ID=test-logs-$(date +%Y%m%d-%H%M%S)
        echo "TEST_RUN_ID not set, using ${TEST_RUN_ID}"
    else
        echo "TEST_RUN_ID is ${TEST_RUN_ID}"
    fi

    local QEMU_CONN="${1}"

    # the current use case assumes that a proxy is always required
    local QEMU_CONN="${1}"
    if [[ ${QEMU_CONN} != "PTY" && ${QEMU_CONN} != "TCP" ]]; then
        QEMU_CONN="TCP"
        echo "QEMU connection: ${QEMU_CONN} (using default)"
    else
        echo "QEMU connection: ${QEMU_CONN}"
        shift
    fi


    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    (
        cd ${WORKSPACE_TEST_FOLDER}

        print_info "Prepare TA integration tests"

        if [ -d ${FOLDER_BUILD_TA} ]; then
            rm -rf ${FOLDER_BUILD_TA}
        fi
        mkdir ${FOLDER_BUILD_TA}

        # copy files from test automation framework to execute them from the
        # test workspace. We do this, because we do not want to pollute the
        # sources, since python creates a folder __pycache__ with "compiled"
        # python scripts at the location of the scripts.
        # ToDo: The copy operation should better happen in the preparation,
        #       step. It's done here for convenience reasons, as it allows
        #       working on a test script and then just executing the "run"
        #       stage, which will use the changed script then
        cp -R ${DIR_SRC_TA}/* ${FOLDER_BUILD_TA}/

        print_info "Prepare KeyStore image"
        # Create a fresh keystore image for each test run.
        sdk_kpt \
            ${DIR_SRC_KPD}/preprovisionedKeys.xml \
            ${FOLDER_BUILD_TA}/tests/preProvisionedKeyStoreImg

        print_info "Running TA integration tests"
        # run tests in sub shell
        (
            cd ${FOLDER_BUILD_TA}/tests

            PYTEST_PARAMS=(
                -v
                # --capture=no   # show printf() from python scripts in console
                --workspace_path=${WORKSPACE_ROOT}

                # even if it's called proxy_path, it the proxy binary actually
                --proxy_path=${ABS_DIR_BIN_SDK}/proxy_app

                # QEMU connection mode (PTY or TCP)
                --qemu_connection=${QEMU_CONN}

                --target=${BUILD_PLATFORM:-"zynq7000"}
                --test_run_id=${TEST_RUN_ID}
                --junitxml=${WORKSPACE_ROOT}/${TEST_RUN_ID}/test_results.xml
            )

            pytest ${PYTEST_PARAMS[@]} $@
        )

    )

    echo "test run complete"
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

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

    run_test $@


else
    echo "invalid parameter, use"
    echo "   build"
    echo "   prepare"
    echo "   run [pytest_params_and_args]"
    exit 1

fi
