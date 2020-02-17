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

WORKSPACE_TEST_DIR=workspace_test


# Test Automation
DIR_SRC_TA=${DIR_SRC}/ta
FOLDER_BUILD_TA=ta


# SEOS Sandbox
DIR_SRC_SANDBOX=${DIR_SRC}/seos_sandbox
DIR_SRC_SEOS_LIBS=${DIR_SRC_SANDBOX}/projects/libs/seos_libs

# Keystore Provisioning Tool
DIR_SRC_KPT=${DIR_SRC}/keystore_provisioning_tool
FOLDER_BUILD_KPT=kpt

# Keystore Provisioning Demo
DIR_SRC_KPD=${DIR_SRC}/src/tests/demo_preprovisioned_keystore

# Proxy
PROXY_SRC=${DIR_SRC}/proxy
FOLDER_BUILD_PROXY=proxy


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

        print_info "Building test plan documentation"
        mkdir -p ${DIR_SRC_TA}/doc
        # run build in subshell
        (
            cd ${DIR_SRC_TA}/doc
            pydoc3 -w ../tests/*.py
        )
        mv ${DIR_SRC_TA}/doc .

        print_info "Building SEOS Libs Unit Tests"
        # run preparation script in sub shell
        (
            ${DIR_SRC_SEOS_LIBS}/test.sh prepare
        )

        print_info "Building Proxy Linux Application"
        mkdir -p ${FOLDER_BUILD_PROXY}
        # run build in subshell
        (
            cd ${FOLDER_BUILD_PROXY}
            ${PROXY_SRC}/build.sh ${DIR_SRC_SANDBOX}
        )

        print_info "Building KeyStore provisioning tool"
        mkdir -p ${FOLDER_BUILD_KPT}
        # run build in subshell
        (
            cd ${FOLDER_BUILD_KPT}
            ${DIR_SRC_KPT}/build.sh ${DIR_SRC_SANDBOX}
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

        # copy files from test automation framework
        if [ -d ${FOLDER_BUILD_TA} ]; then
            rm -rf ${FOLDER_BUILD_TA}
        fi
        mkdir ${FOLDER_BUILD_TA}
        cp -R ${DIR_SRC_TA}/* ${FOLDER_BUILD_TA}/

        # run the pre-provisioning tool and output the prepared binary to
        # the test folder to be used by the provisioning test
        (
            cd ${FOLDER_BUILD_KPT}
            ${DIR_SRC_KPT}/run.sh \
                ${DIR_SRC_KPD}/preprovisionedKeys.xml \
                build/keystore_provisioning_tool  \
                ../${FOLDER_BUILD_TA}/tests/preProvisionedKeyStoreImg
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

            # use a relative paths here, since they are guaranteed to be the
            # same. The absolute paths cab be different depending on the CI
            # slave or in case or parallel builds, which will lead to the test
            # report analyzer thinking these are different tests then.
            local DIR_REL_WORKSPACE_ROOT=$(realpath --relative-to="$(pwd)" "${WORKSPACE_ROOT}")
            local DIR_REL_PROXY=$(realpath --relative-to="$(pwd)" "${WORKSPACE_ROOT}/${WORKSPACE_TEST_DIR}/${FOLDER_BUILD_PROXY}/build/proxy_app")

            PYTEST_PARAMS=(
                -v
                # --capture=no   # show printf() from python scripts in console
                --workspace_path=${DIR_REL_WORKSPACE_ROOT}

                # even if it's called proxy_path, it the proxy binary actually
                --proxy_path=${DIR_REL_PROXY}
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
