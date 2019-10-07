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
SEOS_PROJECTS_DIR="${BUILD_SCRIPT_DIR}/src/tests"


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

    # build SEOS API documentation
    run_build DOC "seos_sandbox_doc" -DSEOS_SANDBOX_DOC=ON $@

    # collect SEOS API documentation
    (
        cd ${BUILD_DIR}
        local DOC_MODULES=$(find . -name html -type d -printf "%P\n")

        # folder where we collect things
        local SEOS_DOC_OUTPUT=SEOS-API_doc-html
        if [[ -e ${SEOS_DOC_OUTPUT} ]]; then
            echo "removing attic SEOS API documentation collection folder"
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

    # build SEOS projects documentation
    (
        cd ${BUILD_DIR}

        SEOS_PROJECTS_DOC_OUTPUT=SEOS-Projects_doc-html
        if [[ -e ${SEOS_PROJECTS_DOC_OUTPUT} ]]; then
            echo "removing attic SEOS projects documentation collection folder"
            rm -rf ${SEOS_PROJECTS_DOC_OUTPUT}
        fi
        mkdir ${SEOS_PROJECTS_DOC_OUTPUT}
        cd ${SEOS_PROJECTS_DOC_OUTPUT}

        # Actually, details should move to each test, so we should just iterate
        # over all the folders and invoke a documentation build there
        SEOS_PROJECTS_DOC_DIRS=(
            test_crypto_api/components/TEST_CRYPTO/src
            test_spiffs_filestream/components/TestSpiffsFileStream/src
            keystore_demo_app/components/DemoApp/src
            pre_provisioned_keystore/components/DemoApp/src
        )

        for project_ctx in ${SEOS_PROJECTS_DOC_DIRS[@]}; do
            local prj_name=${project_ctx%%/*}
            local prj_dir=${project_ctx#*/}
            echo ""
            echo "###"
            echo "### project documentation for: ${prj_name}"
            echo "###"
            mkdir -p ${prj_name}
            (
                export DOXYGEN_INPUT_DIR=${SEOS_PROJECTS_DIR}/${prj_name}/${prj_dir}
                export DOXYGEN_OUTPUT_DIR=${prj_name}
                doxygen ${SEOS_PROJECTS_DIR}/Doxyfile
                cp -ar ${prj_name}/html/* ${prj_name}
                rm -rf ${prj_name}/html
            )
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
        # since the cmake root CMakeList file is in SEOS_SANDBOX_DIR,
        # this must either be relative to it or hold an absolute path
        -DBUILD_PROJECT=${SEOS_PROJECTS_DIR}/${BUILD_PROJECT}
        -DKernelVerificationBuild=OFF
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
    # we could drop this hard coded list and simply use all subdirectories in
    # ${BUILD_SCRIPT_DIR}/src/tests. However, there is a certain order in the
    # list here, starting with the most simple thing, then move on to more
    # complext test systems and finally have the demos. Rationale is, that if
    # the simpler builds fail, there is no point in building more complex
    # things because we likely run into the same problems again.
    ALL_PROJECTS=(
        # tests
        test_hello_world
        test_syslog
        test_crypto_api
        test_picotcp_api
        test_proxy_nvm
        test_spiffs_filestream
        # demos
        keystore_demo_app
        pre_provisioned_keystore
    )

    # for now, just loop over the list above and abort the whole build on the
    # first error. Ideally we would not abort here, but try to do all builds
    # and ten report which failed. Or better, the caller should invoke this
    # build script several times in parallel for each SEOS system.
    for BUILD_PROJECT in ${ALL_PROJECTS[@]}; do
        run_build_mode zynq7000 Debug ${BUILD_PROJECT} $@
    done

    run_astyle
}


#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

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
