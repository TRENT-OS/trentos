cmake_minimum_required(VERSION 3.7.2)

seos_set_config_file(config.h)

seos_use_libs(
    SEOS_LIBS
    SEOS_CRYPTO
    SEOS_CRYPTO_IMPL_MBEDTLS
    SPIFFS
)

project(tests_proxy_nvm C)

DeclareCAmkESComponent(TestSpiffsFileStream
    SOURCES
        "components/TestSpiffsFileStream/src/testSpiffsFileStream.c"
        "components/TestSpiffsFileStream/src/initTest.c"
        "components/TestSpiffsFileStream/src/fileStreamUnitTests.c"
        "components/TestSpiffsFileStream/src/fileStreamDemoTest.c"
    LIBS
        seos_libs
        seos_crypto
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

DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
