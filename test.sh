#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

PROJECT_DIR=$(cd `dirname $0` && pwd)
WORKSPACE_ROOT=$(pwd)

WORKSPACE_TEST_DIR=workspace_test


# Test Automation
TA_FOLDER=ta
TA_SRC_FOLDER=${PROJECT_DIR}/${TA_FOLDER}

# SEOS Sandbox
SEOS_SANDBOX_FOLDER=${PROJECT_DIR}/seos_sandbox
SEOS_LIBS_FOLDER=${SEOS_SANDBOX_FOLDER}/projects/libs/seos_libs

# Keystore Provisioning Tool
KPT_SRC=${PROJECT_DIR}/keystore_provisioning_tool
KPT_BUILD=kpt

# Keystore Provisioning Demo
KPD_SRC=${PROJECT_DIR}/src/tests/demo_preprovisioned_keystore

# Proxy
PROXY_SRC=${PROJECT_DIR}/proxy
PROXY_BUILD=proxy

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

        print_info "Building SEOS Libs Unit Tests"
        # run preparation script in sub shell
        (
            ${SEOS_LIBS_FOLDER}/test.sh prepare
        )

        print_info "Building Proxy Linux Application"
        mkdir -p ${PROXY_BUILD}
        # run build in subshell
        (
            cd ${PROXY_BUILD}
            ${PROXY_SRC}/build.sh ${SEOS_SANDBOX_FOLDER}
        )

        print_info "Building KeyStore provisioning tool"
        mkdir -p ${KPT_BUILD}
        # run build in subshell
        (
            cd ${KPT_BUILD}
            ${KPT_SRC}/build.sh ${SEOS_SANDBOX_FOLDER}
        )
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

        print_info "Check Python version and packages"
        check_tool_installed python3
        check_tool_installed pip3
        check_tool_installed pytest

        # setup a python virtual environment if needed. This is a convenience
        # function for users who want to run the tests locally. In the CI
        # environment the dependencies are usually always there and therefore
        # this functions should always skip the installation.
        local requirements_file="${TA_SRC_FOLDER}/tests/requirements.txt"
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
            ${SEOS_LIBS_FOLDER}/test.sh run
        )

        print_info "Prepare TA integration tests"

        # copy files from test automation framework
        if [ -d ${TA_FOLDER} ]; then
            rm -rf ${TA_FOLDER}
        fi
        mkdir ${TA_FOLDER}
        cp -R ${TA_SRC_FOLDER}/* ${TA_FOLDER}/

        # run the pre-provisioning tool and output the prepared binary to
        # the test folder to be used by the provisioning test
        (
            cd ${KPT_BUILD}
            ${KPT_SRC}/run.sh \
                ${KPD_SRC}/preprovisionedKeys.xml \
                build/keystore_provisioning_tool  \
                ../${TA_FOLDER}/tests/preProvisionedKeyStoreImg
        )

        print_info "Running TA integration tests"

        PYTEST_PARAMS=(
            -v
            # --capture=no   # show printf() from python scripts in console
            --workspace_path="${WORKSPACE_ROOT}"

            # even if it's called proxy_path, it the proxy binary actually
            --proxy_path="${WORKSPACE_ROOT}/${WORKSPACE_TEST_DIR}/${PROXY_BUILD}/build/mqtt_proxy"
        )

        # run tests in sub shell
        (
            cd ${TA_FOLDER}/tests

            # activate python virtual environment if it exists
            if [ -f ${PYTHON_VENV_ACTIVATE} ]; then
                print_info "entering python virtual environment '${PYTHON_VENV_NAME}'"
                source ${PYTHON_VENV_ACTIVATE}
            fi

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
