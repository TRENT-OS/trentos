cmake_minimum_required(VERSION 3.7.2)

seos_set_config_file(config.h)

seos_use_libs(SEOS_LIBS)

project(tests_proxy_nvm C)

DeclareCAmkESComponent(ProxyNVM_Tester1
    INCLUDES
		components/ChanMux/src/instance
    SOURCES
        src/ProxyNVM_Tester1.c
        src/ProxyNVM_common.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

DeclareCAmkESComponent(ProxyNVM_Tester2
    INCLUDES
        components/ChanMux/src/instance
    SOURCES
        src/ProxyNVM_Tester2.c
        src/ProxyNVM_common.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

DeclareCAmkESComponent(UartDrv
    SOURCES
        components/Uart/src/UartDrv.c
        components/Uart/src/qemu_uart.c
    C_FLAGS
        -Wall -Werror
)

DeclareCAmkESComponent(ChanMux
    SOURCES
        components/ChanMux/src/instance/ChanMux_Instance.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

#ROOT SERVER
DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
