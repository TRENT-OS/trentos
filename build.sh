#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019-2022, HENSOLDT Cyber GmbH
#
#-------------------------------------------------------------------------------

BUILD_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"
DIR_SRC_SANDBOX="${BUILD_SCRIPT_DIR}/seos_sandbox"
SDK_OUT_DIR="OS-SDK"
SDK_PKG_OUT_DIR="${SDK_OUT_DIR}/pkg"

# Test settings
DIR_SRC_TA="${BUILD_SCRIPT_DIR}/ta"
WORKSPACE_TEST_FOLDER="workspace_test"


# This list is used for the targets "all-projects" and "all". The order within
# the list starts with the most simple system to build, then moves on to more
# complex test systems and finally has the demos. Rationale is, that if the
# simpler builds fail, there is no point in building more complex things,
# because we likely run into the same problems again. And the building the
# demos does not make any sense if we can't even build the tests.
WELL_KNOWN_PROJECTS=(
    # The entry format is: <project_name>,<folder>[,<test_script>]
    # If <folder> ends with a "/" then <project_name> is appended, as the
    # project is assumed to be in a subfolder with the same name as the project
    # name.

    demo_hello_world,src/demos/,test_demo_hello_world.py

    # native systems, require compiling with -DSDK_USE_CAMKES=0
    native_sel4test,src/native/sel4test,test_native_sel4test.py
    native_sel4bench,src/native/sel4bench,test_native_sel4bench.py
    native_hello_world,src/native/hello_world,test_native_hello_world.py

    # tests
    test_timeserver,src/tests/
    test_crypto_api,src/tests/
    test_certserver,src/tests/
    test_certparser,src/tests/
    test_cryptoserver,src/tests/
    test_entropysource,src/tests/
    test_uart,src/tests/
    test_chanmux,src/tests/
    test_proxy_nvm,src/tests/
    test_filesystem,src/tests/
    test_logserver,src/tests/
    test_config_server,src/tests/
    test_keystore,src/tests/
    test_network_api,src/tests/
    test_storage_interface,src/tests/
    test_tls_api,src/tests/
    test_tlsserver,src/tests/

    # demos
    demo_iot_app,src/demos/,test_demo_iot_app.py
    demo_iot_app_rpi3,src/demos/
    demo_iot_app_imx6,src/demos/
    demo_network_filter,src/demos/
    demo_raspi_ethernet,src/demos/
    demo_tls_api,src/demos/,test_demo_tls_api.py
    demo_i2c,src/demos/
    demo_tls_server,src/demos/
    demo_mailbox_rpi,src/demos/
    demo_vm_minimal,src/demos/,test_demo_vm_minimal.py
    demo_vm_serialserver,src/demos/,test_demo_vm_serialserver.py
    demo_vm_virtio_net,src/demos/,test_demo_vm_virtio_net.py
)


ALL_PROJECTS_EXCLUDE_zynq7000=(
    demo_iot_app_rpi3
    demo_raspi_ethernet
    demo_iot_app_imx6
)

ALL_PROJECTS_EXCLUDE_rpi3=(
    demo_iot_app
)


#-------------------------------------------------------------------------------
function run_system_build()
{
    if [ "$#" -lt 5 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local SDK_DIR=${1}
    local PROJECT_NAME=${2}
    local PROJECT_DIR=${3}
    local BUILD_PLATFORM=${4}
    local BUILD_TYPE=${5}
    shift 5

    # build output will be generated in this folder
    local BUILD_DIR=build-${BUILD_PLATFORM}-${BUILD_TYPE}-${PROJECT_NAME}

    local PARAMS=(
        # build-system.sh expects at least these parameters
        ${PROJECT_DIR}
        ${BUILD_PLATFORM}
        ${BUILD_DIR}
        #------------------------------------------------
        # every parameter below is passed to CMake
        -D CMAKE_BUILD_TYPE=${BUILD_TYPE}
    )

    if [ ! -d ${SDK_DIR} ]; then
        echo "missing SDK in ${SDK_DIR}"
        return 1
    fi

    # Show the actual command line that is executed to start the system build
    # using the SDK package.
    (
        set -x
        ${SDK_DIR}/build-system.sh "${PARAMS[@]}" $@
    )
}


#-------------------------------------------------------------------------------
# Parameters:
#   BUILD_PLATFORM: target platform
#   BUILD_TYPE: Debug/Release
#   PATH_OR_PROJECT: path to project or name from WELL_KNOWN_PROJECTS
function run_sdk_and_system_build()
{
    if [ "$#" -lt 3 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local BUILD_PLATFORM=${1}
    local BUILD_TYPE=${2}
    local PATH_OR_PROJECT="${3}"
    shift 3 # all other params are passed to the project build

    local PROJECT_NAME=""
    local PROJECT_DIR=""
    if [ -d "${PATH_OR_PROJECT}" ]; then
        PROJECT_NAME=$(basename ${PROJECT_DIR})
        PROJECT_DIR="${PATH_OR_PROJECT}"
    else
        for PROJECT_DESCR in ${WELL_KNOWN_PROJECTS[@]}; do
            local PARAM=""
            local PARAMS=()
            while [ "${PROJECT_DESCR}" != "${PARAM}" ] ;do
                # extract the substring from start of string up to delimiter.
                local PARAM=${PROJECT_DESCR%%,*}
                # delete this first "element" AND next separator, from $IN.
                PROJECT_DESCR="${PROJECT_DESCR#$PARAM,}"
                # Print (or doing anything with) the first "element".
                PARAMS+=($PARAM)
            done
            PROJECT_NAME=${PARAMS[0]}
            if [[ "${PATH_OR_PROJECT}" == "${PROJECT_NAME}" ]]; then
                PROJECT_DIR=${PARAMS[1]:-}
                if [[ -z "${PROJECT_DIR}" || "${PROJECT_DIR}" == "-" ]]; then
                    echo "ERROR: no project directory for ${PROJECT_NAME}"
                    print_usage_help
                    exit 1
                fi
                break;
            fi
        done
        if [ -z "${PROJECT_DIR}" ]; then
            echo "ERROR: unknown project: ${PATH_OR_PROJECT}"
            print_usage_help
            exit 1
        fi
        # if PROJECT_DIR ends with "/" then append PROJECT_NAME
        if [[ "${PROJECT_DIR}" =~ ^.*/$ ]]; then
            PROJECT_DIR="${PROJECT_DIR}${PROJECT_NAME}"
        fi
        PROJECT_DIR="${BUILD_SCRIPT_DIR}/${PROJECT_DIR}"
    fi
    echo "Project Name:   ${PROJECT_NAME}"
    echo "Project Folder: ${PROJECT_DIR}"
    if [ "$#" -gt 0 ]; then
        echo "Parameters:     $@"
    fi

    # first build a source-only SDK package from the SDK sources and then use
    # this to build the system. This does not cost much time and ensures we can
    # build a the system with the SDK package. We don't need to build the full
    # package, because no SDK tools or docs are needed to build a system. In
    # case the SDK shall really be used directly, don't call build-sdk.sh and
    # pass ${DIR_SRC_SANDBOX} instead of ${SDK_PKG_OUT_DIR} to run_system_build.
    echo "collecting SDK sources in ${SDK_OUT_DIR}"
    ${DIR_SRC_SANDBOX}/build-sdk.sh collect-sources ${SDK_OUT_DIR}

    local PARAMS=(
        ${SDK_PKG_OUT_DIR}  # ${DIR_SRC_SANDBOX} to use the SDK sources directly
        ${PROJECT_NAME}
        ${PROJECT_DIR}
        ${BUILD_PLATFORM}
        ${BUILD_TYPE}
    )
    run_system_build ${PARAMS[@]} $@

    echo "running astyle check ..."
    (
        cd ${BUILD_SCRIPT_DIR}
        ${DIR_SRC_SANDBOX}/astyle_check_sdk.sh
    )
}


#-------------------------------------------------------------------------------
function build_all_projects()
{
    if [ "$#" -ne 2 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local BUILD_ACTION_SDK=$1
    local BUILD_TYPE=$2
    shift 2

    ALL_PLATFORMS=(
      #  # --- ARM ---
      #  am335x
      #  # am335x-boneblack
      #  # am335x-boneblue
      #  # apq8064
      #  # bcm2837
      #      rpi3
      #  # bcm2711
      #      rpi4
      #  # exynos4
      #  # exynos5 # -> exynos5250
      #  #     exynos5250
      #  #     exynos5410
      #  #     exynos5422
      #  # #fvp    !!!build error
      #  # hikey
      #  # imx6 # -> sabre
      #  #     sabre
      #  #     wandq
      #  # #imx7/imx7sabre, but does not compile)
      #  # imx8mq-evk
      #  # imx8mm-evk
      #  # imx31 # should default to kzm, but does not
      #  #     kzm # imx31
      #  # odroidc2
      #  # omap3
      #  # # qemu-arm-virt  !!!build error
      #  # rockpro64
      #  # tk1
      #  # tx1
      #  # tx2
        zynq7000
      #  zynqmp
      #      # ultra96 #zynqmp, but does not compile
      #
      #  # --- RISC-V ---
      #  # ariane
      #  hifive
      #  # spike
      #
      #  # --- x86 ---
      #  # pc99 # does not compile
    )

    echo "building SDK Package (${BUILD_ACTION_SDK}) in ${SDK_OUT_DIR}"
    ${DIR_SRC_SANDBOX}/build-sdk.sh ${BUILD_ACTION_SDK} ${SDK_OUT_DIR}

    # for now, just loop over the list above and abort the whole build on the
    # first error. Ideally we would not abort here, but try to do all builds
    # and then report which failed. Or better, the caller should invoke this
    # build script several times in parallel for each system.
    for PROJECT_DESCR in ${WELL_KNOWN_PROJECTS[@]}; do
        local PARAM=""
        local PARAMS=()
        while [ "${PROJECT_DESCR}" != "${PARAM}" ] ;do
            # extract the substring from start of string up to delimiter.
            local PARAM=${PROJECT_DESCR%%,*}
            # delete this first "element" AND next separator, from $IN.
            PROJECT_DESCR="${PROJECT_DESCR#$PARAM,}"
            # Print (or doing anything with) the first "element".
            PARAMS+=($PARAM)
        done
        local PROJECT_NAME=${PARAMS[0]}
        local PROJECT_DIR=${PARAMS[1]:-}

        # if there is no project directory then do nothing
        if [[ -z "${PROJECT_DIR}" || "${PROJECT_DIR}" == "-" ]]; then
            continue
        fi
        # if PROJECT_DIR ends with "/" then append PRJ_NAME
        if [[ "${PROJECT_DIR}" =~ ^.*/$ ]]; then
            PROJECT_DIR="${PROJECT_DIR}${PROJECT_NAME}"
        fi
        for BUILD_PLATFORM in ${ALL_PLATFORMS[@]}; do

            eval EXCLUDE_LIST=\${ALL_PROJECTS_EXCLUDE_${BUILD_PLATFORM}[@]}
            local skip=0
            for EXCL_PRJ in ${EXCLUDE_LIST[@]}; do
                if [[ ${EXCL_PRJ} == ${PROJECT_NAME} ]]; then
                    echo -e "\nSkipping excluded project: ${PROJECT_NAME}"
                    skip=1
                    break
                fi
            done

            if [[ ${skip} -ne 0 ]]; then
                break
            fi

            local PARAMS=(
                ${SDK_PKG_OUT_DIR}  # ${DIR_SRC_SANDBOX} to use SDK sources directly
                ${PROJECT_NAME}
                ${BUILD_SCRIPT_DIR}/${PROJECT_DIR}
                ${BUILD_PLATFORM}
                ${BUILD_TYPE}
            )
            run_system_build ${PARAMS[@]} $@

        done
    done

    echo "running astyle check ..."
    (
        cd ${BUILD_SCRIPT_DIR}
        ${DIR_SRC_SANDBOX}/astyle_check_sdk.sh
    )
}


#-------------------------------------------------------------------------------
function do_test_prepare()
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
        echo "##"
        echo "## running SDK build step: ${step}"
        echo "##"
        ${DIR_SRC_SANDBOX}/build-sdk.sh ${step} "${WORKSPACE_TEST_FOLDER}/${SDK_OUT_DIR}"
    done

    echo "##"
    echo "### OS SDK build complete"
    echo "##"
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
        echo "##"
        echo "## Building test plan documentation"
        echo "##"
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
# Params: BUILD_PLATFORM [pytest params ...]
function run_tests()
{
    if [ "$#" -lt 1 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local BUILD_PLATFORM=$1
    shift

    # the workspace holds the SDK+tools, temporary files and partition images
    if [ ! -d ${WORKSPACE_TEST_FOLDER} ]; then
        echo "ERROR: missing test workspace"
        exit 1
    fi

    # There must be an SDK package in the workspace. We don't check details
    # about the the content here, because it depends on a specific test what is
    # needed, thus tests must take care of such details.
    local DIR_PKG_SDK=${WORKSPACE_TEST_FOLDER}/${SDK_PKG_OUT_DIR}
    if [ ! -d ${DIR_PKG_SDK} ]; then
        echo "ERROR: missing SDK package"
        exit 1
    fi

    # process command line parameters.
    local TEST_SCRIPTS=()
    local TEST_PARAMS=()
    for param in $@; do
        # everything that starts with a dash is considered a parameter,
        # otherwise it's considered a script name.
        if [[ ${param} =~ ^-.*$  ]]; then
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

    for TEST_SCRIPT in ${TEST_SCRIPTS}; do
        local TEST_SCRIPT_BASENAME=$(basename ${TEST_SCRIPT})
        local PROJECT_NAME="${TEST_SCRIPT_BASENAME%.*}"
        # derive test or demo project folder from our naming convention
        local PROJECT_SRC_DIR="${PROJECT_NAME::4}s/${PROJECT_NAME}"

        # find well known project by test script name
        for PROJECT_DESCR in ${WELL_KNOWN_PROJECTS[@]}; do
            local PARAM=""
            local PARAMS=()
            while [ "${PROJECT_DESCR}" != "${PARAM}" ] ;do
                # extract the substring from start of string up to delimiter.
                local PARAM="${PROJECT_DESCR%%,*}"
                # delete this first "element" AND next separator, from $IN.
                PROJECT_DESCR="${PROJECT_DESCR#$PARAM,}"
                # Print (or doing anything with) the first "element".
                PARAMS+=($PARAM)
            done
            local PRJ_NAME="${PARAMS[0]}"
            local PRJ_TEST_SCRIPT="${PARAMS[2]:-}"
            if [[ -z "${PRJ_TEST_SCRIPT}" ]]; then
                PRJ_TEST_SCRIPT="${PRJ_NAME}.py"
            fi
            if [[ "${PRJ_TEST_SCRIPT}" == "${TEST_SCRIPT}" ]]; then
                PROJECT_NAME=${PRJ_NAME}
                local PROJECT_DIR="${PARAMS[1]:-}"
                # if PROJECT_DIR ends with "/" then append PRJ_NAME
                if [[ "${PROJECT_DIR}" =~ ^.*/$ ]]; then
                    PROJECT_DIR="${PROJECT_DIR}${PROJECT_NAME}"
                fi
                PROJECT_SRC_DIR="${PROJECT_DIR}"
                break;
            fi
        done

        local BUILD_FOLDER="build-${BUILD_PLATFORM}-Debug-${PROJECT_NAME}"
        local TEST_SYSTEM_LOG_DIR="${TEST_LOGS_DIR}/${TEST_SCRIPT_BASENAME}"

        echo "##=============================================================================="
        echo "## running test"
        echo "##   Project Name:           ${PROJECT_NAME}"
        echo "##   Project Source Folder:  ${PROJECT_SRC_DIR:-n/a}"
        echo "##   Project Build Folder:   ${BUILD_FOLDER}"
        echo "##   Logs:                   ${TEST_SYSTEM_LOG_DIR}"
        echo "##------------------------------------------------------------------------------"

        # create the folder to run the test in and collect all output and test
        # setup files in
        mkdir -p "${TEST_SYSTEM_LOG_DIR}"

        # if the project provides a test preparation script, execute this prior
        # to the test run
        if [[ -z "${PROJECT_SRC_DIR}" ]]; then
            local ABS_TEST_SYSTEM_SETUP="${BUILD_SCRIPT_DIR}/src/${PROJECT_SRC_DIR}/prepare_test.sh"
            if [ -f "${ABS_TEST_SYSTEM_SETUP}" ]; then
                (
                    ABS_DIR_PGK_SDK="$(realpath ${DIR_PKG_SDK})"
                    cd "${TEST_SYSTEM_LOG_DIR}"
                    echo "Running test preparation script..."
                    ${ABS_TEST_SYSTEM_SETUP} ${ABS_DIR_PGK_SDK}
                )
            fi
        fi

        if [ "${BUILD_PLATFORM}" == "jetson-xavier-nx-dev-kit" ] || [ "${BUILD_PLATFORM}" == "aetina-an110-xnx" ]; then
            SYSTEM_IMAGE=$(realpath ${BUILD_FOLDER}/images/os_image.binary)
        else
            SYSTEM_IMAGE=$(realpath ${BUILD_FOLDER}/images/os_image.elf)
        fi

        PYTHON_PARAMS=(
            -B  # do not create *.pyc files
            -m pytest  # execute pytest
            #--------------------------------------------------
            # all parameters below are fed into pytest
            #--------------------------------------------------
            -p no:cacheprovider  # don't create .cache directories
            -v  # increase pytest verbosity (-vv is even more verbose)
            -o log_cli=True  # write logs to console
            --capture=no  # show printf() from pytest scripts in console
            --tb=short  # control the traceback (long, short, line, native, no)
            #--collect-only  # show test, but don't run them
            #--exitfirst  # exit on first test error
            --junitxml=$(realpath ${TEST_LOGS_DIR})/test_results.xml
            #--------------------------------------------------
            # test framework parameters
            #--------------------------------------------------
            --print_logs  # show log output from device in console
            --target=${BUILD_PLATFORM}
            --system_image=$(realpath ${SYSTEM_IMAGE})
            --proxy=$(realpath ${DIR_PKG_SDK}/bin/proxy_app)
            --log_dir=$(realpath ${TEST_LOGS_DIR})
            # --sd_card=536870912  # 512 MiB
            # ToDo: The resources are taken from the SDK package and not from
            #       the current sandbox that might be used for development. This
            #       leads to the inconvenient effect that resources changes are
            #       not accessible for the tests unless do_test_prepare() is run
            #       again.
            --resource_dir=$(realpath ${DIR_PKG_SDK}/resources)
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
# Params: [params ...]
function run_build_and_test()
{
    if [ "$#" -lt 1 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local PATH_OR_PROJECT="${1}"

    local TEST_SCRIPT=""

    for PROJECT_DESCR in ${WELL_KNOWN_PROJECTS[@]}; do
        local PARAM=""
        local PARAMS=()
        while [ "${PROJECT_DESCR}" != "${PARAM}" ] ;do
            # extract the substring from start of string up to delimiter.
            local PARAM=${PROJECT_DESCR%%,*}
            # delete this first "element" AND next separator, from $IN.
            PROJECT_DESCR="${PROJECT_DESCR#$PARAM,}"
            # Print (or doing anything with) the first "element".
            PARAMS+=($PARAM)
        done
        PROJECT_NAME=${PARAMS[0]}
        if [[ "${PATH_OR_PROJECT}" == "${PROJECT_NAME}" ]]; then
            local TEST_SCRIPT=${PARAMS[2]:-}
            if [[ -z "${TEST_SCRIPT}" ]]; then
                TEST_SCRIPT="${PROJECT_NAME}.py"
            fi
        fi
    done

    if [[ -z "${TEST_SCRIPT}" ]]; then
        TEST_SCRIPT="$(basename ${PATH_OR_PROJECT}).py"
    fi

    ${DIR_SRC_SANDBOX}/scripts/open_trentos_build_env.sh $0 "$@"
    ${DIR_SRC_SANDBOX}/scripts/open_trentos_test_env.sh $0 test-run ${TEST_SCRIPT}
}


#-------------------------------------------------------------------------------
function print_usage_help()
{
    echo ""
    echo "Usage: build.sh <target> ..."
    echo ""
    echo " Options for <target> are:"
    echo "   sdk [action, defaults to 'all']   (SDK package build)"
    echo "   all                               (SDK package and all known projects)"
    echo "   all-projects                      (just the projects)"
    echo "   clean                             (delete all build output folders)"
    echo "   test-prepare                      (prepare test workspace)"
    echo "   test-doc                          (create test docs)"
    echo "   test-run [params]                 (run test)"
    echo "   build-and-test <project> [params] (build with params and run test)"
    echo "   <folder>                          (folder with a project)"
    echo "   <project name>                    (well known project name)"
    echo ""
}

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

DEFAULT_BUILD_TYPE="Debug"
#DEFAULT_BUILD_TYPE="Release"
#DEFAULT_BUILD_TYPE="RelWithDebInfo"
#DEFAULT_BUILD_TYPE="MinSizeRel"

DEFAULT_BUILD_PLATFORM="zynq7000"
#DEFAULT_BUILD_PLATFORM="sabre"
#DEFAULT_BUILD_PLATFORM="nitrogen6sx"
#DEFAULT_BUILD_PLATFORM="zynqmp"
#DEFAULT_BUILD_PLATFORM="rpi3"
#DEFAULT_BUILD_PLATFORM="rpi4"
#DEFAULT_BUILD_PLATFORM="hikey"
#DEFAULT_BUILD_PLATFORM="odroidc2"
#DEFAULT_BUILD_PLATFORM="odroidc4"
#DEFAULT_BUILD_PLATFORM="fvp"
#DEFAULT_BUILD_PLATFORM="hifive"
#DEFAULT_BUILD_PLATFORM="polarfire"
#DEFAULT_BUILD_PLATFORM="spike32"
#DEFAULT_BUILD_PLATFORM="spike64"
#DEFAULT_BUILD_PLATFORM="qemu-arm-virt"
#DEFAULT_BUILD_PLATFORM="qemu-arm-virt-a15"
#DEFAULT_BUILD_PLATFORM="qemu-arm-virt-a53"
#DEFAULT_BUILD_PLATFORM="qemu-arm-virt-a57"
#DEFAULT_BUILD_PLATFORM="qemu-arm-virt-a72"
#DEFAULT_BUILD_PLATFORM="qemu-riscv-virt32"
#DEFAULT_BUILD_PLATFORM="qemu-riscv-virt64"
#DEFAULT_BUILD_PLATFORM="ia32"
#DEFAULT_BUILD_PLATFORM="x86_64"
#DEFAULT_BUILD_PLATFORM="jetson-nano-2gb-dev-kit"
#DEFAULT_BUILD_PLATFORM="jetson-tx2-nx-a206"
#DEFAULT_BUILD_PLATFORM="jetson-xavier-nx-dev-kit"
#DEFAULT_BUILD_PLATFORM="aetina-an110-xnx"



BUILD_TYPE=${BUILD_TYPE:-${DEFAULT_BUILD_TYPE}}
BUILD_PLATFORM=${BUILD_PLATFORM:-${DEFAULT_BUILD_PLATFORM}}

case "${1:-}" in

    "help"|"")
        print_usage_help
        exit 1
        ;;

    "sdk")
        shift
        BUILD_ACTION_SDK=${1:-all}
        if [ "$#" -gt 1 ]; then
            shift
        fi
        echo "building SDK Package (${BUILD_ACTION_SDK}) in ${SDK_OUT_DIR}"
        ${DIR_SRC_SANDBOX}/build-sdk.sh ${BUILD_ACTION_SDK} ${SDK_OUT_DIR}
        ;;

    "all-projects")
        shift
        # build SDK source-only package and use this to build all projects
        build_all_projects collect-sources ${BUILD_TYPE} $@
        ;;

    "all")
        shift
        # build SDK package with binaries and use this to build all projects
        build_all_projects all ${BUILD_TYPE} $@
        ;;

    "clean")
        rm -rf ${SDK_OUT_DIR}
        rm -rf build-*
        ;;

    "test-prepare")
        shift
        do_test_prepare
        ;;

    "test-doc")
        shift
        build_test_plan_docs
        ;;

    "test-run")
        shift
        run_tests ${BUILD_PLATFORM} $@
        ;;

    "build-and-test")
        shift
        run_build_and_test $@
        ;;

    *)
        # if we are here, $@ has at least one parameter, which is either a
        # project folder or a name from WELL_KNOWN_PROJECTS
        run_sdk_and_system_build ${BUILD_PLATFORM} ${BUILD_TYPE} $@
        ;;
esac
