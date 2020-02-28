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

# This list is used for the targets "all-projects" and "all". The order within
# the list starts with the most simple system to build, then moves on to more
# complex test systems and finally has the demos. Rationale is, that if the
# simpler builds fail, there is no point in building more complex things,
# because we likely run into the same problems again. And the building the
# demos does not make any sense if we can't even build the tests.
WELL_KNOWN_PROJECTS=(
    # the entry format is <test name>,<folder>. We don't make the fixed assumption
    # that the folder name matched the test name or that there is a generic
    # subfolder where all tests are. This assumption holds today, but there is
    # a plan to drop using "src/tests" if the whole test system is a GIT
    # submodule and we don't have any local sources.

    # tests
    test_hello_world,src/tests/test_hello_world
    test_config_server,src/tests/test_config_server
    test_config_server_fs_backend,src/tests/test_config_server_fs_backend
    # test_filesystem_api,- # ToDo: we need a test project
    test_crypto_api,src/tests/test_crypto_api
    test_tls_api,src/tests/test_tls_api
    test_keystore,src/tests/test_keystore
    # test_network_api,src/tests/test_network_api
    test_proxy_nvm,src/tests/test_proxy_nvm
    # test_seos_filestream,src/tests/test_seos_filestream
    test_chanmux,src/tests/test_chanmux
    test_partition_manager,src/tests/test_partition_manager
    test_filesystem_as_lib,src/tests/test_filesystem_as_lib
    test_logserver,src/tests/test_logserver

    # demos
    # demo_keystore,src/tests/demo_keystore
    # demo_preprovisioned_keystore,src/tests/demo_preprovisioned_keystore
    # demo_fs_as_components,src/tests/demo_fs_as_components
    # demo_fs_as_libs,src/tests/demo_fs_as_libs
    # demo_configuration_as_lib,src/tests/demo_configuration_as_lib
    # demo_configuration_as_component,src/tests/demo_configuration_as_component
)


#-------------------------------------------------------------------------------
function map_project()
{
    local VAR_PRJ_DIR=${1}
    local TEST_SYSTEM=${2:-}

    for PROJECT in ${WELL_KNOWN_PROJECTS[@]}; do
        local PRJ_NAME=${PROJECT%,*}
        local PRJ_DIR=${PROJECT#*,}

        if [[ "${TEST_SYSTEM}" == "${PRJ_NAME}" ]]; then

            if [[ "${PRJ_DIR}" == "-" ]]; then
                echo "ERROR: no project directory for ${PRJ_NAME}"
                exit 1
            fi

            eval "${VAR_PRJ_DIR}=${PRJ_DIR}"
            return 0
        fi

    done

    return 1
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

            cmake $@ -G Ninja ${SEOS_SANDBOX_DIR}

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
        cp -ar seos-api-index.html ${SEOS_DOC_OUTPUT}/index.html
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
            # format: <test project from WELL_KNOWN_PROJECTS>[:<doc root>]
            # test_crypto_api:components/TEST_CRYPTO/src
            # test_keystore
            # test_seos_filestream # can build doc from use project root
            # demo_keystore:components/DemoApp/src
            # demo_preprovisioned_keystore:components/DemoApp/src
        )

        cat <<EOF >>index.html
<!doctype html>
<html>
  <head></head>
  <body>
    <ul>
      <li>Tests
        <ul>
          <li><a href="test_crypto_api/index.html">Crypto</a></li>
          <li><a href="test_keystore/index.html">Keystore</a></li>
          <li><a href="test_seos_filestream/index.html">SEOS Filestream</a></li>
        </ul>
      </li>
      <li>Demos
        <ul>
          <li><a href="hello_world_demo_app/index.html">Hallo World</a>
          <li><a href="demo_keystore/index.html">Keystore</a></li>
          <li><a href="demo_preprovisioned_keystore/index.html">Preprovisioned Keystore</a></li>
          <li><a href="http_demo_app/index.html">HTTP Demo</a></li>
        </ul>
      </li>
    </ul>
  </body>
</html>
EOF

        for PROJECT in ${SEOS_PROJECTS_DOC_DIRS[@]}; do
            local PRJ_NAME=${PROJECT/:*/}
            local PRJ_DOC_DIR=""
            if [[ "${PROJECT}" != "${PRJ_NAME}" ]]; then
                PRJ_DOC_DIR=${PROJECT/*:/}
            fi

            if ! map_project MAPPED_PROJECT_DIR ${PRJ_NAME}; then
                echo "ERROR: unknown project ${PRJ_NAME}"
                exit 1
            fi

            if [[ "${MAPPED_PROJECT_DIR}" == "-" ]]; then
                echo "ERROR: no project directory for ${PRJ_NAME}"
                exit 1
            fi

            echo ""
            echo "###"
            echo "### creating project documentation for ${PRJ_NAME}"
            local DOC_ROOT=${MAPPED_PROJECT_DIR}
            if [[ ! -z "${PRJ_DOC_DIR}" ]]; then
                DOC_ROOT+=/${PRJ_DOC_DIR}
            fi
            echo "### doc root:  ${DOC_ROOT}"

            mkdir -p ${PRJ_NAME}
            (
                export DOXYGEN_INPUT_DIR=${BUILD_SCRIPT_DIR}/${DOC_ROOT}
                export DOXYGEN_OUTPUT_DIR=${PRJ_NAME}
                doxygen ${BUILD_SCRIPT_DIR}/Doxyfile
                cp -ar ${PRJ_NAME}/html/* ${PRJ_NAME}
                rm -rf ${PRJ_NAME}/html
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

    local BUILD_PROJECT_DIR=${1}
    local BUILD_PLATFORM=${2}
    local BUILD_TYPE=${3}
    shift 3

    local BUILD_PROJECT_NAME=$(basename ${BUILD_PROJECT_DIR})
    local TARGET_NAME=${BUILD_PLATFORM}-${BUILD_TYPE}-${BUILD_PROJECT_NAME}

    local CMAKE_PARAMS=(
        # settings processed by CMake directly
        -DCMAKE_TOOLCHAIN_FILE=${SEOS_SANDBOX_DIR}/kernel/gcc.cmake
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
        # seL4 build system settings
        # SEL4_CACHE_DIR is a binary cache. There are some binaries (currently
        # musllibc and capDL-toolthat) that project agnostic, so we don't have
        # to rebuild them every time. This reduces the build time a lot.
        -DSEL4_CACHE_DIR=$(pwd)/cache-${BUILD_PLATFORM}
        -DPLATFORM=${BUILD_PLATFORM}
        -DKernelVerificationBuild=OFF
        # SEOS build system settings
        -DSEOS_PROJECT_DIR=${BUILD_SCRIPT_DIR}
        -DSEOS_SYSTEM=${BUILD_PROJECT_DIR}
    )

    case "${BUILD_PLATFORM}" in
        #-------------------------------------
        am335x | am335x-boneblack | am335x-boneblue | \
        apq8064 |\
        bcm2837 | rpi3 | bcm2837-rpi3 |\
        exynos4 |\
        exynos5 | exynos5250 | exynos5410 | exynos5422 |\
        hikey |\
        imx6 | sabre | imx6-sabre | wandq | imx6-wandq |\
        imx7  | imx7-sabre |\
        imx31 | kzm | imx31-kzm |\
        omap3 |\
        qemu-arm-virt |\
        tk1 |\
        zynq7000 )

            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=arm-linux-gnueabi-
            )
            ;;
        #-------------------------------------
        fvp  |\
        imx8mq-evk | imx8mm-evk |\
        odroidc2 |\
        rockpro64 |\
        tx1 |\
        tx2 |\
        zynqmp | zynqmp-zcu102 | zynqmp-ultra96 | ultra96 )
            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=aarch64-linux-gnu-
            )
            ;;
        #-------------------------------------
        ariane |\
        hifive |\
        spike )
            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=riscv64-unknown-linux-gnu-
            )
            ;;
        #-------------------------------------
        pc99)
            CMAKE_PARAMS+=(
                -DCROSS_COMPILER_PREFIX=x86_64-linux-gnu-
            )
            ;;
        #-------------------------------------
        *)
            echo "invalid target: ${BUILD_TARGET}"
            exit 1
            ;;
    esac

    run_build ${TARGET_NAME} all ${CMAKE_PARAMS[@]} $@
}


#-------------------------------------------------------------------------------
function build_all_projects()
{
    ALL_PLATFORMS=(
      #  # --- ARM ---
      #  am335x
      #  # am335x-boneblack
      #  # am335x-boneblue
      #  # apq8064
      #  # bcm2837
      #      rpi3
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

    # for now, just loop over the list above and abort the whole build on the
    # first error. Ideally we would not abort here, but try to do all builds
    # and then report which failed. Or better, the caller should invoke this
    # build script several times in parallel for each SEOS system.
    for PROJECT in ${WELL_KNOWN_PROJECTS[@]}; do
        local PRJ_NAME=${PROJECT%,*}
        local PRJ_DIR=${PROJECT#*,}

        if [[ "${PRJ_DIR}" != "-" ]]; then
            for BUILD_PLATFORM in ${ALL_PLATFORMS[@]}; do
                run_build_mode ${PRJ_DIR} ${BUILD_PLATFORM} Debug $@
            done
        fi
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

elif map_project MAPPED_PROJECT_DIR $@; then
    echo "building ${1:-} from ${MAPPED_PROJECT_DIR} ..."
    shift
    run_build_mode ${MAPPED_PROJECT_DIR} zynq7000 Debug $@
    run_astyle

elif [ ! -z $@ ]; then
    BUILD_PROJECT_DIR=${1:-}
    shift
    echo "building ${BUILD_PROJECT_DIR} using params: '$@' ..."
    run_build_mode ${BUILD_PROJECT_DIR} zynq7000 Debug $@
    run_astyle

else
    echo -e "build.sh <target> [cmake options]\
    \n\npossible targets are:\
    \n\t doc (documentation)\
    \n\t all\
    \n\t all-projects (everything but the documentation)\
    \n\t check_astyle_artifacts (to be run after a build, it tries to find astyle artifacts indicating discrepancies with the coding standards)\
    \n\t clean\
    \n\t <TEST_DIR> (folder with a test project)
    \n\t <TEST_NAME> (well known name of a test project)"
    exit 1
fi
