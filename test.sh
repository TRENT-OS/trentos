#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------


TEST_SCRIPT_DIR=$(cd `dirname $0` && pwd)
WORKSPACE_ROOT=$(pwd)

TEST_DIR=workspace_test

PROXY_FOLDER=mqtt_proxy_demo
TA_FOLDER=ta


#-------------------------------------------------------------------------------
function do_clone()
{
    if [ "$#" -ne 2 ]; then
        echo "ERROR: not enough parameters"
        return 1
    fi

    local REPO_URL=ssh://git@bitbucket.hensoldt-cyber.systems:7999/${1}.git
    local FOLDER=${2}
    shift 2

    # check which local branch we are on and try to check this branch out from
    # the other repos also. If there is not then use master. Since we support
    # out-of-source builds (and tests), we can't blindly check the current
    # folder, but have to check the folder where this script is in.
    BRANCH=`git -C ${TEST_SCRIPT_DIR} describe --contains --all HEAD | cut -d/ -f3`
    RET=0
    git ls-remote --exit-code ${REPO_URL} ${BRANCH} || RET=$?
    if [ ${RET} != 0 ]; then
        echo "no dedicated branch exists, will use master"
        BRANCH=master
    fi
    git clone --recursive -b ${BRANCH} ${REPO_URL} ${FOLDER}
}


#-------------------------------------------------------------------------------
function prepare_test()
{

    # remove folder if it exists already. This should not happen in CI when we
    # have a clean workspace, but it's convenient for local builds
    if [ -d ${TEST_DIR} ]; then
        rm -rf ${TEST_DIR}
    fi

    (
        mkdir ${TEST_DIR}
        cd ${TEST_DIR}

        # get and build the proxy
        (
            do_clone hc/mqtt_proxy_demo ${PROXY_FOLDER}/src
            cd ${PROXY_FOLDER}
            src/build.sh
        )

        # get and build the test automation framework
        (
            do_clone hc/ta ${TA_FOLDER}
            cd ${TA_FOLDER}
            python3 -m venv ta-env
            source ta-env/bin/activate
            pip install -r requirements.txt
        )

    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
function run_test()
{
    (
        cd ${TEST_DIR}/${TA_FOLDER}
        source ta-env/bin/activate

        cd seos_tests

        PYTEST_PARAMS=(
            -v
            -s
            --workspace_path="${WORKSPACE_ROOT}"

            # even if it's called proxy_path, it the proxy binary actually
            --proxy_path="${WORKSPACE_ROOT}/${TEST_DIR}/${PROXY_FOLDER}/build/mqtt_proxy"
        )

        pytest ${PYTEST_PARAMS[@]}


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

    run_test

else
    echo "invalid parameter, use \"prepare\" or \"run\""
    exit 1
fi
