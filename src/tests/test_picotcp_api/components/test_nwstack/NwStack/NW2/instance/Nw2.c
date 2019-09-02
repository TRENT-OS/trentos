/*
 *  SEOS Network Stack CAmkES wrapper
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include <camkes.h>
#include <string.h>
#include "LibDebug/Debug.h"
#include "SeosNwStack.h"


int run()
{
    Debug_LOG_INFO("starting network stack as Server...\n");
    int ret;

    seos_nw_camkes_signal_glue  nw_signal =
    {
        .e_write_emit        =  e_write_2_emit,
        .c_write_wait        =  c_write_2_wait,
        .e_read_emit         =  e_read_2_emit,
        .c_read_wait         =  c_read_2_wait,
        .e_conn_emit         =  e_conn_2_emit,
        .c_conn_wait         =  c_conn_2_wait,
        .e_write_nwstacktick =  e_write_nwstacktick_2_emit,
        .c_nwstacktick_wait  =  c_nwstacktick_2_wait,
        .e_initdone          =  e_initdone_2_emit,
        .c_initdone          =  c_initdone_2_wait
    };

    seos_nw_ports_glue nw_data =
    {
        .ChanMuxDataPort = chanMuxDataPort_2,
        .ChanMuxCtrlPort = chanMuxCtrlDataPort_2,
        .Appdataport     = NwAppDataPort_2

    };

    Seos_nw_camkes_info nw_camkes =
    {
        &nw_signal,
        &nw_data,
        SEOS_NWSTACK_AS_SERVER
    };


    // should never return as this starts pico_stack_tick().

    ret = Seos_NwStack_init(&nw_camkes);

    /*is possible when proxy does not run with use_tap=1 param. Just print and exit*/
    if (ret != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Network Stack Init()-Sever Failed...Exiting NwStack-2. Error:%d\n",
                          ret);
    }
    return 0;
}
