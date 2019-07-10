cmake_minimum_required(VERSION 3.7.2)

if (SEOS_LIBS)
    add_subdirectory(seos_libs)
endif()

if (SEOS_MQTT)
    add_subdirectory(seos_mqtt)
endif()

