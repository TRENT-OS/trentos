#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

DIR_SRC=$(cd `dirname $0` && pwd)
WORKSPACE_ROOT=$(pwd)


#-------------------------------------------------------------------------------
# our workspace name
WORKSPACE_TEST_DIR=workspace_test


#-------------------------------------------------------------------------------
# SEOS Sandbox
DIR_SRC_SANDBOX=${DIR_SRC}/seos_sandbox
DIR_SRC_SEOS_LIBS=${DIR_SRC_SANDBOX}/projects/libs/seos_libs

# Keystore Provisioning Tool
FOLDER_BUILD_KPT=kpt

# Proxy
FOLDER_BUILD_PROXY=proxy


#-------------------------------------------------------------------------------
# Test Automation
DIR_SRC_TA=${DIR_SRC}/ta
FOLDER_BUILD_TA=ta


#-------------------------------------------------------------------------------
# Keystore Provisioning Demo
DIR_SRC_KPD=${DIR_SRC}/src/tests/demo_preprovisioned_keystore


#-------------------------------------------------------------------------------
# Python virtual environment
PYTHON_VENV_NAME=ta-env
PYTHON_VENV_ACTIVATE=${PYTHON_VENV_NAME}/bin/activate


#-------------------------------------------------------------------------------
function print_info()
{
    local INFO=$1

    echo -e "\n\n############ ${INFO}\n"
}


#-------------------------------------------------------------------------------
function build_seos_sdk_tool()
{
    local SDK_TOOLS_SRC_DIR=$1
    local FOLDER_BUILD=$2

    print_info "Building SDK tool: ${SDK_TOOLS_SRC_DIR}"

    if [ ! -d ${FOLDER_BUILD} ]; then
        mkdir -p ${FOLDER_BUILD}
    fi

    # run build in subshell
    (
        cd ${FOLDER_BUILD}
        ${DIR_SRC_SANDBOX}/${SDK_TOOLS_SRC_DIR}/build.sh ${DIR_SRC_SANDBOX}
    )
}


#-------------------------------------------------------------------------------
function build_test_tools()
{
    # remove folder if it exists already. This should not happen in CI when we
    # have a clean workspace, but it's convenient for local builds
    if [ -d ${WORKSPACE_TEST_DIR} ]; then
        rm -rf ${WORKSPACE_TEST_DIR}
    fi
    mkdir ${WORKSPACE_TEST_DIR}

    (
        cd ${WORKSPACE_TEST_DIR}

        build_seos_sdk_tool tools/proxy ${FOLDER_BUILD_PROXY}

        build_seos_sdk_tool tools/keystore_provisioning_tool ${FOLDER_BUILD_KPT}
        cp -v ${DIR_SRC_SANDBOX}/tools/keystore_provisioning_tool/{xmlParser.py,run.sh} ${FOLDER_BUILD_KPT}
    )

    echo "test tool building complete"
}

#-------------------------------------------------------------------------------
function check_tool_installed()
{
    local TOOL=$1
    which ${TOOL} > /dev/null || { echo "${TOOL} is required, please install it" && exit 1; }
}

#-------------------------------------------------------------------------------
function prepare_test()
{
    if [ ! -d ${WORKSPACE_TEST_DIR} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    (
        cd ${WORKSPACE_TEST_DIR}

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

        print_info "Building SEOS Libs Unit Tests"
        # run preparation script in sub shell
        (
            ${DIR_SRC_SEOS_LIBS}/test.sh prepare
        )


        print_info "Check Python version and packages"
        check_tool_installed python3
        check_tool_installed pip3
        check_tool_installed pytest

        # setup a python virtual environment if needed. This is a convenience
        # function for users who want to run the tests locally. In the CI
        # environment the dependencies are usually always there and therefore
        # this functions should always skip the installation.
        local requirements_file="${DIR_SRC_TA}/tests/requirements.txt"
        local installed=$(pip3 freeze)
        local required=$(cat ${requirements_file})
        local missing_pkg=""

        for pkg in ${required}; do
            if [[ ! ${installed} =~ ${pkg} ]]; then
                missing_pkg=${pkg}
                break
            fi
        done

        if [ ! -z "${missing_pkg}" ] ; then
            print_info "Missing python package: '${missing_pkg}'"
            print_info "Creating virtual environment '${PYTHON_VENV_NAME}'"
            python3 -m venv ${PYTHON_VENV_NAME}
            source ${PYTHON_VENV_ACTIVATE}
            pip3 install -r ${requirements_file}
        fi
    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
function run_test()
{
    if [ ! -d ${WORKSPACE_TEST_DIR} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    (
        cd ${WORKSPACE_TEST_DIR}

        print_info "Running SEOS Libs Unit Tests"
        # run seos_libs unit tests in sub shell
        (
            ${DIR_SRC_SEOS_LIBS}/test.sh run
        )

        print_info "Prepare TA integration tests"

        if [ -d ${FOLDER_BUILD_TA} ]; then
            rm -rf ${FOLDER_BUILD_TA}
        fi
        mkdir ${FOLDER_BUILD_TA}

        # copy files from test automation framework to execute them from the
        # test workspace. We do this, because we do not want to pollute the
        # sources, since python creates a folder __pycache__ with "compiled"
        # python scripts at the location of the scripts.
        # ToDo: The copy operaction should better happen in the preparation,
        #       step. It's done here for convenince reasons, as it allows
        #       working on a test script and then just executing the "run"
        #       stage, which will use the changed script then
        cp -R ${DIR_SRC_TA}/* ${FOLDER_BUILD_TA}/

        print_info "Prepare KeyStore image"
        # run the pre-provisioning tool and output the prepared binary to
        # the test folder to be used by the provisioning test
        (
            ${FOLDER_BUILD_KPT}/run.sh \
                ${DIR_SRC_KPD}/preprovisionedKeys.xml \
                ${FOLDER_BUILD_KPT}/build/keystore_provisioning_tool  \
                ${FOLDER_BUILD_TA}/tests/preProvisionedKeyStoreImg
        )

        print_info "Running TA integration tests"
        # run tests in sub shell
        (
            cd ${FOLDER_BUILD_TA}/tests

            # activate python virtual environment if it exists
            if [ -f ${PYTHON_VENV_ACTIVATE} ]; then
                print_info "entering python virtual environment '${PYTHON_VENV_NAME}'"
                source ${PYTHON_VENV_ACTIVATE}
            fi

            if [ -z "${TEST_RUN_ID:-}" ]; then
                local TEST_RUN_ID=test-logs-$(date +%Y%m%d-%H%M%S)
                echo "TEST_RUN_ID not set, using ${TEST_RUN_ID}"
            else
                echo "TEST_RUN_ID is ${TEST_RUN_ID}"
            fi

            PYTEST_PARAMS=(
                -v
                # --capture=no   # show printf() from python scripts in console
                --workspace_path=${WORKSPACE_ROOT}

                # even if it's called proxy_path, it the proxy binary actually
                --proxy_path=${WORKSPACE_ROOT}/${WORKSPACE_TEST_DIR}/${FOLDER_BUILD_PROXY}/build/proxy_app

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

    build_test_tools


elif [[ "${1:-}" == "prepare" ]]; then
    shift

    # ToDo: the step "build_test_tools" is executed here to keep backwards
    #       compatibility. It will be removed once all script have been updated
    #       to invoke the build step above.
    build_test_tools
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
