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
PROVISIONING_TOOL_FOLDER=keystore_provisioning_tool
SEOS_LIBS_FOLDER=${PROJECT_DIR}/seos_sandbox/projects/libs/seos_libs


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

        # prepare seos_libs unit tests
        (
            echo -e "\n\n############ Building SEOS Libs Unit Tests ################\n"
            ${SEOS_LIBS_FOLDER}/test.sh prepare
        )

        # get and build the proxy
        (
            echo -e "\n\n############## Building Proxy Linux Application ################\n"
            mkdir -p ${PROXY_FOLDER}/src && cp -R ${PROJECT_DIR}/${PROXY_FOLDER}/* ${PROXY_FOLDER}/src/
            cd ${PROXY_FOLDER}
            src/build.sh
        )

        # copy files from test automation framework
        (
            mkdir ${TA_FOLDER} && cp -R ${PROJECT_DIR}/${TA_FOLDER}/* ${TA_FOLDER}/
        )

        # get and build the keystore provisioning tool and prepare the keystore image
        (
            echo -e "\n\n############## Building KeyStore provisioning tool ################\n"
            mkdir -p ${PROVISIONING_TOOL_FOLDER}/src && cp -R ${PROJECT_DIR}/${PROVISIONING_TOOL_FOLDER}/* ${PROVISIONING_TOOL_FOLDER}/src/
            cd ${PROVISIONING_TOOL_FOLDER}
            ./src/build.sh ./src ./build ${PROJECT_DIR}/seos_sandbox
            #run the pre-provisioning tool and output the prepared binary to the test
            #folder to be used by the provisioning test
            ./src/run.sh ./src/keysExample.xml ./build/tool_build/src/keystore_provisioning_tool ../ta/tests/preProvisionedKeyStoreImg
        )
    )

    echo "test preparation complete"
}


#-------------------------------------------------------------------------------
function run_test()
{
    echo -e "\n\n############## Running SEOS Libs Unit Tests ################\n"
    # run seos_libs unit tests
    (
        cd ${TEST_DIR}
        ${SEOS_LIBS_FOLDER}/test.sh run
    )

    echo -e "\n\n############## Running TA integration tests  ###############\n"
    (
        cd ${TEST_DIR}/${TA_FOLDER}
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
