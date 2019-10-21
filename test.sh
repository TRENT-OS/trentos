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

PROXY_FOLDER=proxy
TA_FOLDER=ta
PROVISIONING_TOOL_FOLDER=keystore_provisioning_tool
SEOS_LIBS_FOLDER=${PROJECT_DIR}/seos_sandbox/projects/libs/seos_libs
VENV_NAME="ta-env"

#-------------------------------------------------------------------------------
function check_pytest_requirements_and_install_if_needed()
{
    ### This function is for conveninency of a user that wants to run tests
    ### locally. In CI environment the depenencies are usually always there
    ### and therefore the function should alway skip the intallation part there.

    local requirements_file="tests/requirements.txt"
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
        echo "#### Creating '${VENV_NAME}' virtual environement."
        python3 -m venv ${VENV_NAME}
        source ${VENV_NAME}/bin/activate
        pip3 install -r ${requirements_file}
    fi
}


#-------------------------------------------------------------------------------
function prepare_test()
{
    # check python3 packages
    which python3 > /dev/null || { echo "python3 is required, please install it" && exit 1; }
    which pip3    > /dev/null || { echo "python3-pip is required, please install it" && exit 1; }

    # remove folder if it exists already. This should not happen in CI when we
    # have a clean workspace, but it's convenient for local builds
    if [ -d ${WORKSPACE_TEST_DIR} ]; then
        rm -rf ${WORKSPACE_TEST_DIR}
    fi
    mkdir ${WORKSPACE_TEST_DIR}

    (
        cd ${WORKSPACE_TEST_DIR}

        echo -e "\n\n############ Building SEOS Libs Unit Tests ################\n"
        # run preparation script in sub shell
        (
            ${SEOS_LIBS_FOLDER}/test.sh prepare
        )

        echo -e "\n\n############## Building Proxy Linux Application ################\n"
        mkdir -p ${PROXY_FOLDER}/src
        cp -R ${PROJECT_DIR}/${PROXY_FOLDER}/* ${PROXY_FOLDER}/src/
        # run build in subshell
        (
            cd ${PROXY_FOLDER}
            src/build.sh
        )

        echo -e "\n\n############## Preparing TA scripts environment ################\n"
        # copy files from test automation framework
        mkdir ${TA_FOLDER}
        cp -R ${PROJECT_DIR}/${TA_FOLDER}/* ${TA_FOLDER}/
        cd ${TA_FOLDER}
        # setup a python virtual environment if needed
        check_pytest_requirements_and_install_if_needed

        # get and build the keystore provisioning tool and prepare the keystore image
        echo -e "\n\n############## Building KeyStore provisioning tool ################\n"
        mkdir -p ${PROVISIONING_TOOL_FOLDER}/src
        cp -R ${PROJECT_DIR}/${PROVISIONING_TOOL_FOLDER}/* ${PROVISIONING_TOOL_FOLDER}/src/
        (
            cd ${PROVISIONING_TOOL_FOLDER}
            # build tool
            src/build.sh \
                src \
                build \
                ${PROJECT_DIR}/seos_sandbox
            # run the pre-provisioning tool and output the prepared binary to
            # the test folder to be used by the provisioning test
            src/run.sh \
                src/keysExample.xml \
                build/tool_build/src/keystore_provisioning_tool \
                ../ta/tests/preProvisionedKeyStoreImg
        )
    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
function run_test()
{
    (
        cd ${WORKSPACE_TEST_DIR}

        echo -e "\n\n############## Running SEOS Libs Unit Tests ################\n"
        # run seos_libs unit tests in sub shell
        (
            ${SEOS_LIBS_FOLDER}/test.sh run
        )

        echo -e "\n\n############## Running TA integration tests  ###############\n"


        PYTEST_PARAMS=(
            -v
            # --capture=no   # show printf() from python scripts in console
            --workspace_path="${WORKSPACE_ROOT}"

            # even if it's called proxy_path, it the proxy binary actually
            --proxy_path="${WORKSPACE_ROOT}/${WORKSPACE_TEST_DIR}/${PROXY_FOLDER}/build/mqtt_proxy"
        )

        # run tests in sub shell
        (
            cd ${TA_FOLDER}/tests

            # activate python virtual environment if it exists
            if [ -f ${VENV_NAME}/bin/activate ]; then
                source ${VENV_NAME}/bin/activate
            fi

            pytest ${PYTEST_PARAMS[@]} $@
        )

    )

    echo "test run complete"
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

if [[ "${1:-}" == "prepare" ]]; then
    shift

    prepare_test


elif [[ "${1:-}" == "run" ]]; then
    shift

    run_test $@

else
    echo "invalid parameter, use \"prepare\" or \"run [pytest_params_and_args] \""
    exit 1
fi
