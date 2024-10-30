# TRENTOS

Welcome to the TRENTOS project. 

TRENTOS (Trusted Entity Operating System) is an embedded OS build upon the seL4 microkernel and CAmkES

## Requirements

* Linux (We recommend Ubuntu 20.04 or newer)
* Docker or Podman

## Getting Started

Get started with TRENTOS in 5 simple steps:
```sh
# 1. Create a new folder for TRENTOS
mkdir trentos && cd trentos

# 2. Clone the TRENTOS repository
git clone --recursive git@github.com:TRENT-OS/trentos.git src

# 3. Pull the TRENTOS Test & Build Docker container from Docker Hub
docker pull hensoldtcyber/trentos_build:latest
docker pull hensoldtcyber/trentos_test:latest

# 4. Prepare the test environment (only needs to be done once)
src/build.sh test-prepare

# 5. Build & execute demo_hello_world
src/build.sh build-and-test demo_hello_world
```

[TRENTOS Getting Started](https://trent-os.github.io/trentos/introduction/getting-started.html)

## Documentation

Please refer to our [TRENTOS Docs](https://trent-os.github.io/trentos).

## Supported Platforms

Currently TRENTOS 

* zynq7000
* sabre
* nitrogen6sx
* zynqmp
* rpi3
* rpi4
* hikey
* odroidc2
* odroidc4
* fvp
* hifive
* polarfire
* spike32
* spike64
* qemu-arm-virt
* qemu-arm-virt-a15
* qemu-arm-virt-a53
* qemu-arm-virt-a57
* qemu-arm-virt-a72
* qemu-riscv-virt32
* qemu-riscv-virt64
* ia32
* x86_64
* jetson-nano-2gb-dev-kit
* jetson-tx2-nx-a206
* jetson-xavier-nx-dev-kit
* aetina-an110-xnx

<!-- TODO: Curate list and give small comment to current status of platform-->
<!-- TODO: (Maybe) Change names or add comment which is better telling -->
<!-- TODO: Maybe seperate this into its own more detailed page -->


## TRENTOS Demos

Current demonstrators included in the standard TRENTOS source tree:

* demo_iot_app
* demo_iot_app_hw
* demo_network_filter
* demo_raspi_ethernet
* demo_tls_api
* demo_i2c
* demo_tls_server
* demo_mailbox_rpi
* demo_vm_minimal
* demo_vm_serialserver
* demo_vm_virtio_net

```sh
src/build.sh build-and-test <name of demo>
```

Even more demos are available as [submodules](https://github.com/orgs/TRENT-OS/repositories?q=demo).

_Please ensure that the selected `DEFAULT_BUILD_PLATFORM` in `trentos/build.sh` is compatible with the selected demonstrator._

## Contributing

Contributions are always welcome. 

To get started please read our [CONTRIBUTING.md](./CONTRIBUTING.md).


## Docker Images

Our Docker images are hosted on [DockerHub](https://hub.docker.com/orgs/hensoldtcyber/repositories). <br />
Dockerfiles are hosted in [Docker Images](https://github.com/TRENT-OS/docker_images).

## Licensing

TRENTOS is available under the OSS GPLv2-or-later license.
Upon request commercial licensing options are available.
Please contact: [info.cyber@hensoldt.net](mailto:info.cyber@hensoldt.net?subject=TRENTOS:%20Commercial%20Licensing) for more information.

A select few components are only available under different licensing terms.
Due to the microkernel concept this may not be an issue as long as these are used as a distinct [TRENTOS components](https://trent-os.github.io/trentos/introduction/camkes.html).

## Testing

TRENTOS comes with a testframework built with pytest and can be found at [`trentos/ta/tests/`](https://github.com/TRENT-OS/tests).

Tests exists for all [tests](https://github.com/orgs/TRENT-OS/repositories?q=test) and for most [demos](https://github.com/orgs/TRENT-OS/repositories?q=demo).

Examle usage: Building and testing `demo_hello_world`:
```sh
src/build.sh build-and-test demo_hello_world
```


## Dependencies

TRENTOS builds upon varies open source libraries and projects:

* sel4
* muslc
* utils
* sel4muslcsys
* sel4platsupport
* sel4utils
* sel4debug
* sel4allocman
* camkes
* capdl
* And more...
