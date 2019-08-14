#!/bin/bash -eu

TEST_NAME=${1:-}
if [ -z ${TEST_NAME} ]; then
    echo "ERROR: missing test name"
    exit 1
fi

# default is the zynq7000 platform
IMAGE_PATH=build-zynq7000-Debug-${TEST_NAME}/images/capdl-loader-image-arm-zynq7000
if [ ! -f ${IMAGE_PATH} ]; then
    echo "ERROR: missing test image ${IMAGE_PATH}"
    exit 1
fi

QEMU_PARAMS=(
    -machine xilinx-zynq-a9
    -m size=512M
    -nographic
    -S   # freeze on startup to allow proxy to connect to PTY
    -s
    -serial pty
    -serial mon:stdio
    -kernel ${IMAGE_PATH}
)

qemu-system-arm  ${QEMU_PARAMS[@]}
