cmake_minimum_required(VERSION 3.7.2)

project(tests_hello_world C)

DeclareCAmkESComponent(MAIN
    INCLUDES
        # no include paths needed
    SOURCES
        src/main.c
    C_FLAGS
        -Wall -Werror
    LIBS
        # no libs needed
)

DeclareCAmkESRootserver(main.camkes)
GenerateCAmkESRootServer()
GenerateSimulateScript()
