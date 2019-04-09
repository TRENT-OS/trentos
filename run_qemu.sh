#!/bin/bash -e

IMAGE_PATH=build-zynq7000-Debug/images/capdl-loader-image-arm-zynq7000

QEMU_PARAMS=(
    -machine xilinx-zynq-a9
    -m size=512M
    -nographic
    -s
    -serial pty
    -serial mon:stdio
    -kernel $IMAGE_PATH
)

qemu-system-arm  ${QEMU_PARAMS[@]}
