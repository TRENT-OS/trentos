cmake_minimum_required(VERSION 3.7.2)

if (TEST_CRYPTO_API)
    include(test_crypto_api/CMakeLists.txt)
else()
    include(test_hello_world/CMakeLists.txt)
endif()
