# TRENTOS

Welcome to the TRENTOS project. 

TRENTOS (Trusted Entity Operating System) is an embedded OS build upon the seL4 microkernel and CAmkES

## Requirements

* Linux (We recommend Ubuntu 20.04 or newer)
* Docker

## Getting Started

Get started with TRENTOS in 5 simple steps:
```sh
# 1. Create a new folder for TRENTOS
mkdir trentos_workspace && cd trentos_workspace

# 2. Clone the TRENTOS repository
git clone --recursive git@github.com:TRENT-OS/trentos.git

# 3. Pull the TRENTOS Test & Build Docker container from Docker Hub
TODO: Add command to pull images

# 4. Prepare the test environment (only needs to be done once)
src/build.sh test-prepare

# 5. Build & execute demo_hello_world
src/build.sh build-and-test demo_hello_world
```

TODO: Link getting started documentation page

## Documentation

TODO: ADD TRENTOS DOCS LINK

Please refer to our [TRENTOS Docs]().

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

TODO: Curate list and give small comment to current status of platform
TODO: (Maybe) Change names or add comment which is better telling
TODO: Maybe seperate this into its own more detailed page


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
trentos/build.sh build-and-test <name of demo>
```

Even more demos are available as [submodules](https://github.com/orgs/TRENT-OS/repositories?q=demo).

## Contributing

Contributions are always welcome. 
To get started please read our [CONTRIBUTING.md](./CONTRIBUTING.md).

If you have any questions or encounter any bugs, please do not hesitate to open an **issue**.


## Licensing

TRENTOS is available under the OSS GPLv2-or-later license.
Upon request commercial licensing options are available.
Please contact: [info.cyber@hensoldt.net](mailto:info.cyber@hensoldt.net?subject=TRENTOS:%20Commercial%20Licensing)
 for more information.

Some few components are only available und different licensing terms. 
Due to the microkernel concept this should not be an issue as long as these are used as a TRENTOS component.

TODO: Link documentation explaining the seL4 component concept.

## Testing

Please refer to ...

TODO: Link wiki page explaining the test framework


## Dependencies

TRENTOS builds upon varies open source libraries:

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
