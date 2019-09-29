cmake_minimum_required(VERSION 3.7.2)

seos_set_config_file(config.h)

seos_use_libs(
    SEOS_LIBS
    SEOS_LIBS_LOGGER
    SEOS_CRYPTO
    SEOS_CRYPTO_IMPL_MBEDTLS
    SEOS_KEYSTORE
    SPIFFS
)

project(tests_crypto_api C)

DeclareCAmkESComponent(TEST_CRYPTO
    INCLUDES
        common
    SOURCES
        components/TEST_CRYPTO/src/testRunner.c
        components/TEST_CRYPTO/src/testSignatureRsa.c
        components/TEST_CRYPTO/src/testKeyStore.c
        components/TEST_CRYPTO/src/testCrypto.c
        components/TEST_CRYPTO/src/testAgreement.c
        common/KeyStoreInit.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_crypto
        seos_keystore
)

DeclareCAmkESComponent(CRYPTO
    INCLUDES
        common
    SOURCES
        components/CRYPTO/instance/Crypto.c
        ${SEOS_LIBS_DIR}/seos_crypto/src/SeosCryptoRpc.c
        ${SEOS_LIBS_DIR}/seos_keystore/src/SeosKeyStoreRpc.c
        common/KeyStoreInit.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_crypto
        seos_keystore
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
