/*
 *  SEOS Netwrok Stack CAmkES wrapper
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
    int instance=1; /* For server */


    nw_camkes_glue nw_inst_1 =
            {
               .e_write_emit        =  e_write_2_emit,
               .e_read_emit         =  e_read_2_emit,
               .e_conn_emit         =  e_conn_2_emit,
               .e_write_nwstacktick =  e_write_nwstacktick_2_emit,
               .e_initdone          =  e_initdone_2_emit,
               .c_write_wait        =  c_write_2_wait,
               .c_read_wait         =  c_read_2_wait,
               .c_conn_wait         =  c_conn_2_wait,
               .c_nwstacktick_wait  =  c_nwstacktick_2_wait,
            };

    nw_chanmux_ports_glue nw_data_1 =
    {
            .ChanMuxDataPort = chanMuxDataPort_2,
            .ChanMuxCtrlPort = chanMuxCtrlDataPort_2,
            .Appdataport     = NwAppDataPort_2

    };


    // should never return as this starts pico_stack_tick().

    ret = Seos_NwStack_init(instance,&nw_inst_1, &nw_data_1);

    if(ret<0)  // is possible when proxy does not run with use_tap =1 param. Just print and exit
    {
        Debug_LOG_INFO("Network Stack Init()-Sever Failed...Exiting NwStack-2\n");
    }
    return 0;
}

