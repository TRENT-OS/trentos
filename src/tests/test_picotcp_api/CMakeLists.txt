cmake_minimum_required(VERSION 3.7.2)

seos_set_config_file(config.h)

seos_use_libs(
    SEOS_LIBS
    SEOS_NETWORK_DRIVER_TAP_PROXY
    SEOS_NW_STACK
)

project(tests_picotcp_api C)

DeclareCAmkESComponent(NwStack
    SOURCES
        components/test_nwstack/NwStack/NW1/instance/Nw.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_nw_lib
        seos_nw_driver_tap_proxy_lib
)

DeclareCAmkESComponent(NwStack_2
    SOURCES
        components/test_nwstack/NwStack/NW2/instance/Nw2.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_nw_lib
        seos_nw_driver_tap_proxy_lib
)

DeclareCAmkESComponent(NwApp
    INCLUDES
        components/test_nwstack/test_APP/include
    SOURCES
        components/test_nwstack/test_APP/test_NwApp/src/Client_App.c
        ${SEOS_LIBS_DIR}/seos_nwstack/network-client-lib/seos_nw_api_interface.c
    C_FLAGS
        -Wall -Werror
        -DCLIENT_CONFIG=1
    LIBS
        seos_core_api
        seos_libs
)

DeclareCAmkESComponent(NwApp_2
    INCLUDES
        components/test_nwstack/test_APP/include
    SOURCES
        components/test_nwstack/test_APP/test_NwApp2/src/Server_App.c
        ${SEOS_LIBS_DIR}/seos_nwstack/network-client-lib/seos_nw_api_interface.c
    C_FLAGS
        -Wall -Werror
        -DSERVER_CONFIG=1
    LIBS
        seos_core_api
        seos_libs
)

DeclareCAmkESComponent(ChanMux
    SOURCES
        components/test_nwstack/ChanMux/src/instance/ChanMux_Instance.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

DeclareCAmkESComponent(TimerClient
    SOURCES
        components/test_nwstack/TimerClient/src/Timer_Client.c
    C_FLAGS
        -Wall -Werror
)

DeclareCAmkESComponent(Timer
    SOURCES
        components/test_nwstack/Timer/src/timer.c
    C_FLAGS
        -Wall -Werror
)

DeclareCAmkESComponent(UartDrv
    SOURCES
        components/test_nwstack/Uart/src/UartDrv.c
        components/test_nwstack/Uart/src/qemu_uart.c
    C_FLAGS
        -Wall -Werror
)

DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
