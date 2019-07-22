/*
 *  SEOS Netwrok Stack CAmkES wrapper
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include <camkes.h>
#include <string.h>
#include "LibDebug/Debug.h"


extern int Seos_NwStack_init();

int run()
{
    Debug_LOG_INFO("starting network stack...\n");
    int ret;

    //ret = NwStack_seos_init();  // should never return as this starts pico_stack_tick().

    ret = Seos_NwStack_init();

    if(ret<0)  // is possible when proxy does not run with use_tap =1 param. Just print and exit
    {
        Debug_LOG_INFO("Network Stack Init() Failed...Exiting NwStack\n");
    }
    return 0;
}

