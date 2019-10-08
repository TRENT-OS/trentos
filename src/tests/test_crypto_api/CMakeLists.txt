cmake_minimum_required(VERSION 3.7.2)

seos_set_config_file(config.h)

seos_use_libs(
    SEOS_LIBS
    SEOS_LIBS_LOGGER
    SEOS_CRYPTO
    SEOS_CRYPTO_IMPL_MBEDTLS
    SPIFFS
)

project(tests_crypto_api C)

DeclareCAmkESComponent(TEST_CRYPTO
    INCLUDES
        common
    SOURCES
        components/TEST_CRYPTO/src/testRunner.c
        components/TEST_CRYPTO/src/testSignatureRsa.c
        components/TEST_CRYPTO/src/testCrypto.c
        components/TEST_CRYPTO/src/testAgreement.c
        components/TEST_CRYPTO/src/testKey.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_crypto
)

DeclareCAmkESComponent(CRYPTO
    INCLUDES
        common
    SOURCES
        components/CRYPTO/instance/Crypto.c
        ${SEOS_LIBS_DIR}/seos_crypto/src/SeosCryptoRpc.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
        seos_crypto
)

DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
