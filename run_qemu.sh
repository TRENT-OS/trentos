#!/bin/bash -e

TEST_NAME=$1
IMAGE_PATH=build-zynq7000-Debug-${TEST_NAME}/images/capdl-loader-image-arm-zynq7000

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
