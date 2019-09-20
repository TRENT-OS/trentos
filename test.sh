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

TEST_DIR=workspace_test

PROXY_FOLDER=proxy
TA_FOLDER=ta


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
            mkdir -p ${PROXY_FOLDER}/src && cp -R $PROJECT_DIR/${PROXY_FOLDER}/* ${PROXY_FOLDER}/src/
            cd ${PROXY_FOLDER}
            src/build.sh
        )

        # get and build the test automation framework
        (
            mkdir ${TA_FOLDER} && cp -R $PROJECT_DIR/${TA_FOLDER}/* ${TA_FOLDER}/
            cd ${TA_FOLDER}
            python3 -m venv ta-env
            source ta-env/bin/activate
            pip install -r tests/requirements.txt
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

        cd tests

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
