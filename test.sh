#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Test script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

SOURCE_DIR=$(cd `dirname $0` && pwd)
WORKSPACE_ROOT=$(pwd)

TEST_DIR=workspace_test

PROXY_FOLDER=proxy
TA_FOLDER=ta
PROVISIONING_TOOL_FOLDER=keystore_provisioning_tool


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
            mkdir -p ${PROXY_FOLDER}/src && cp -R ${SOURCE_DIR}/${PROXY_FOLDER}/* ${PROXY_FOLDER}/src/
            cd ${PROXY_FOLDER}
            src/build.sh
        )

        # get and build the test automation framework
        (
            mkdir ${TA_FOLDER} && cp -R ${SOURCE_DIR}/${TA_FOLDER}/* ${TA_FOLDER}/
            cd ${TA_FOLDER}
            python3 -m venv ta-env
            source ta-env/bin/activate
            pip install -r tests/requirements.txt
        )

        # get and build the keystore provisioning tool and prepare the keystore image
        (
            mkdir -p ${PROVISIONING_TOOL_FOLDER}/src && cp -R ${SOURCE_DIR}/${PROVISIONING_TOOL_FOLDER}/* ${PROVISIONING_TOOL_FOLDER}/src/
            cd ${PROVISIONING_TOOL_FOLDER}
            ./src/build.sh
            #run the pre-provisioning tool and output the prepared binary to the test
            #folder to be used by the provisioning test
            ./src/run.sh ./src/keysExample.xml ./build/src/keystore_provisioning_tool ../ta/tests/preProvisionedKeyStoreImg
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
