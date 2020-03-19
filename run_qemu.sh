#!/bin/bash -eu

QEMU_CONN=${1:-}

if [[ ${QEMU_CONN} != "PTY" && ${QEMU_CONN} != "TCP" ]]; then
    echo "No QEMU connection was set."
else 
    echo "QEMU connection was set to $QEMU_CONN."
    shift 
fi

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

# set no communication as default
SERIAL_COMM="-serial /dev/null"

# change default communication to PTY
if [[ ${QEMU_CONN} == "PTY" ]]; then
    SERIAL_COMM="-S -serial pty"
# change default communication to TCP
elif [[ ${QEMU_CONN} == "TCP" ]]; then
    SERIAL_COMM="-serial tcp:localhost:4444,server"
fi

QEMU_PARAMS=(
    -machine xilinx-zynq-a9
    -m size=512M
    -nographic
    ${SERIAL_COMM}
    -serial mon:stdio
    -kernel ${IMAGE_PATH}
)

qemu-system-arm  ${QEMU_PARAMS[@]}