cmake_minimum_required(VERSION 3.7.2)

find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)
if (DOXYGEN_FOUND)
    add_custom_target( seos_tests_doc
        COMMAND
            export DOXYGEN_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
            && ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation for tests with Doxygen"
        VERBATIM )
else(DOXYGEN_FOUND)
    message("Doxygen needs to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)

if (TEST_CRYPTO_API)
    include(test_crypto_api/CMakeLists.txt)
elseif (TEST_SYSLOG)
    include(test_syslog/CMakeLists.txt)
elseif (TEST_PROXY_NVM)
    include(test_proxy_nvm/CMakeLists.txt)
elseif (TEST_SPIFFS_INTEGRATION)
    include(test_spiffs_integration/CMakeLists.txt)
elseif (TEST_PICOTCP_API)
    include(test_picotcp_api/CMakeLists.txt)
else()
    include(test_hello_world/CMakeLists.txt)
endif()
