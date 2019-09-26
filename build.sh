#!/bin/bash -ue

#-------------------------------------------------------------------------------
#
# Build script
#
# Copyright (C) 2019, Hensoldt Cyber GmbH
#
#-------------------------------------------------------------------------------

BUILD_SCRIPT_DIR=$(cd `dirname $0` && pwd)

SEOS_SANDBOX_DIR="${BUILD_SCRIPT_DIR}/seos_sandbox"


#-------------------------------------------------------------------------------
function prepare_layout()
{
    # long all folder from src into seso_sandbox, so we can use the sanbox
    # CMake build system and it will find everything from src

    local SRC_DIR="${BUILD_SCRIPT_DIR}/src"
    files=`ls ${SRC_DIR}`
    for file in ${files}; do
        ln -sf ${SRC_DIR}/${file} ${SEOS_SANDBOX_DIR}/projects
    done
}


#-------------------------------------------------------------------------------
function clean_layout()
{
    # long all folder from src into seso_sandbox, so we can use the sanbox
    # CMake build system and it will find everything from src

    local SRC_DIR="${BUILD_SCRIPT_DIR}/src"
    files=`ls ${SRC_DIR}`
    for file in ${files}; do
        rm -f ${SEOS_SANDBOX_DIR}/projects/${file}
    done
}


#-------------------------------------------------------------------------------
function run_astyle()
{
    echo "##"
    echo "## running astyle check ..."
    echo "##"

    # ensure there are not existing astyle files anywhere
    find . -name '*.astyle' -exec rm -v {} \;

    # there should be an astyle script in the root folder, but we also search
    # recursively in all subfolders, as modules can come with their own version
    # of the script
    find . -name 'astyle_check.sh' -printf 'running %p\n' -execdir {} \;
}


#-------------------------------------------------------------------------------
function check_astyle_artifacts()
{
    # *.astyle files are generated when file are not astyle compliant. Check if
    # any such file exists in the workspace and fail in this case.
    local files=$(find . -name '*.astyle')
    if [ ! -z "${files}" ]; then
        echo "ERROR: source is not astyle compliant, check: "
        for file in ${files}; do
            echo "  ${file}"
        done
        exit 1
    fi
}

#-------------------------------------------------------------------------------
function run_build()
{
    if [ "$#" -lt 2 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local TARGET_NAME=${1}
    local NINJA_TARGETS=${2}
    shift 2

    echo ""
    echo "##"
    echo "## building: ${TARGET_NAME}"
    echo "##"

    # build dir will be a subdirectory of the current directory, where this
    # script is invoked in. We make this a global variable, so all following
    # steps can find the build directory easily
    BUILD_DIR=$(pwd)/build-${TARGET_NAME}

    # check if cmake init has failed previously
    if [[ -e ${BUILD_DIR} ]] && [[ ! -e ${BUILD_DIR}/rules.ninja ]]; then
        echo "deleting broken build folder and re-initialize it"
        rm -rf ${BUILD_DIR}
    fi

    if [[ ! -e ${BUILD_DIR} ]]; then
        # use subshell to configure the build
        (
            mkdir -p ${BUILD_DIR}
            cd ${BUILD_DIR}

            CMAKE_PARAMS=(
                -DCMAKE_TOOLCHAIN_FILE=${SEOS_SANDBOX_DIR}/kernel/gcc.cmake
                -DKernelVerificationBuild=OFF
            )

            cmake ${CMAKE_PARAMS[@]} $@ -G Ninja ${SEOS_SANDBOX_DIR}

            # must run cmake multiple times, so config settings propagate properly
            echo "re-run cmake (1/2)"
            cmake .
            echo "re-run cmake (2/2)"
            cmake .
        )
    fi

    # build in subshell
    (
        cd ${BUILD_DIR}
        ninja ${NINJA_TARGETS}
    )
}


#-------------------------------------------------------------------------------
function run_build_doc()
{
    # the documentaton build still uses the full seL4/CAmkES build system, so
    # there must be some actual project. Let's use the most simple one.
    run_build DOC "seos_sandbox_doc seos_tests_doc" \
              -DSEOS_SANDBOX_DOC=ON -DBUILD_PROJECT=test_hello_world $@

    (
        cd ${BUILD_DIR}
        local DOC_MODULES=$(find . -name html -type d -printf "%P\n")

        # folder where we collect things
        local SEOS_DOC_OUTPUT=SEOS-doc-html
        if [[ -e ${SEOS_DOC_OUTPUT} ]]; then
            echo "removing attic documentation collection folder"
            rm -rf ${SEOS_DOC_OUTPUT}
        fi
        mkdir ${SEOS_DOC_OUTPUT}
        echo "collecting HTML documentation in ${SEOS_DOC_OUTPUT}..."
        for module in ${DOC_MODULES[@]}; do
            local TARGET_FOLDER=$(basename $(dirname ${module}))
            echo "  ${TARGET_FOLDER} <- ${module}"
            cp -ar ${module} ${SEOS_DOC_OUTPUT}/${TARGET_FOLDER}
        done
    )
}


#-------------------------------------------------------------------------------
function run_build_mode()
{
    if [ "$#" -lt 3 ]; then
        echo "ERROR: invalid parameters for ${FUNCNAME[0]}()"
        return 1
    fi

    local BUILD_TARGET=${1}
    local BUILD_TYPE=${2}
    local BUILD_PROJECT=${3}
    shift 3

    # the tests use hard-coded folder names, where the project name is all
    # upper case. We have to remain compatible and luckily, bash supports "^^"
    # as modifier for variables to make them all upper case letters.
    local TARGET_NAME=${BUILD_TARGET}-${BUILD_TYPE}-${BUILD_PROJECT^^}

    local CMAKE_PARAMS=(
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
        -DBUILD_PROJECT=${BUILD_PROJECT}
    )

    case "${BUILD_TARGET}" in
        #-------------------------------------
        zynq7000)
            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=arm-linux-gnueabi-
                -DKernelARMPlatform=${BUILD_TARGET}
            )
            ;;
        #-------------------------------------
        spike)
            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=riscv64-unknown-linux-gnu-
                -DKernelRiscVPlatform=${BUILD_TARGET}
                -DKernelArch=riscv
                -DKernelRiscVSel4Arch=riscv64
            )
            ;;
        #-------------------------------------
        *)
            echo "invalid target: ${BUILD_TARGET}"
            exit 1
    esac

    run_build ${TARGET_NAME} all ${CMAKE_PARAMS[@]} $@
}


#-------------------------------------------------------------------------------
function build_all_projects()
{
    # we could drop this can simply use the list of sub-folders that exist in
    # ${BUILD_SCRIPT_DIR}/src/tests/ besides the folder "camkes"
    ALL_PROJECTS=(
        # tests
        test_hello_world
        test_syslog
        test_crypto_api
        test_picotcp_api
        test_proxy_nvm
        test_spiffs_integration
        # demos
        keystore_demo_app
        pre_provisioned_keystore
    )

    for BUILD_PROJECT in ${ALL_PROJECTS[@]}; do
        run_build_mode zynq7000 Debug ${BUILD_PROJECT} $@
    done

    run_astyle
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

prepare_layout

if [[ "${1:-}" == "doc" ]]; then
    shift
    run_build_doc $@

elif [[ "${1:-}" == "all-projects" ]]; then
    shift
    build_all_projects $@

elif [[ "${1:-}" == "all" ]]; then
    shift
    run_build_doc $@
    build_all_projects $@

elif [[ "${1:-}" == "check_astyle_artifacts" ]]; then
    shift

    # astyle failures do not abort the build normally, but in CI we don't allow
    # them to happen. Thus this step runs after the build step and we allows
    # differentiate build failures from astyle failures easily then.
    check_astyle_artifacts

elif [[ "${1:-}" == "clean" ]]; then
    shift

    /bin/rm -rf build-*
    clean_layout

else

    if [ ! -z $@ ]; then
        run_build_mode zynq7000 Debug $@
        run_astyle
    else
        echo -e "build.sh <target> [cmake options]\
        \n\npossible targets are:\
        \n\t doc (documentation)\
        \n\t all\
        \n\t all-projects (everything but the documentation)\
        \n\t check_astyle_artifacts (to be run after a build, it tries to find astyle artifacts indicating discrepancies with the coding standards)\
        \n\t clean\
        \n\t TEST_NAME (the name of a test image project under 'tests' folder)"
    fi
fi
