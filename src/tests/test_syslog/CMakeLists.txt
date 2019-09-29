cmake_minimum_required(VERSION 3.7.2)


seos_set_config_file(config.h)

seos_use_libs(
    SEOS_LIBS
    SEOS_LIBS_LOGGER
)


project(tests_syslog C)

DeclareCAmkESComponent(MAIN
    INCLUDES
        components/StdOutLogSpooler/src
    SOURCES
        components/MAIN/src/main.c
        components/MAIN/src/CamkesLogger.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

DeclareCAmkESComponent(StdOutLogSpooler
    INCLUDES
        components/StdOutLogSpooler/src
    SOURCES
        components/StdOutLogSpooler/src/StdOutLogSpooler.c
    C_FLAGS
        -Wall -Werror
    LIBS
        seos_libs
)

DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
