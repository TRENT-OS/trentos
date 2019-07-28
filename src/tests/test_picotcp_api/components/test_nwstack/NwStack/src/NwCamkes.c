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
    Debug_LOG_INFO("starting network stack as Client...\n");
    int ret;
    int instance = 0; /* For client */
    nw_camkes_glue nw_inst_0 =
            {
               .e_write_emit        =  e_write_emit,
               .e_read_emit         =  e_read_emit,
               .e_conn_emit         =  NULL,
               .e_write_nwstacktick =  e_write_nwstacktick_emit,
               .e_initdone          =  e_initdone_emit,
               .c_write_wait        =  c_write_wait,
               .c_read_wait         =  c_read_wait,
               .c_conn_wait         =  NULL,
               .c_nwstacktick_wait  =  c_nwstacktick_wait,
            };
    nw_chanmux_ports_glue nw_data_0 =
    {
     .ChanMuxDataPort     =  chanMuxDataPort,
     .ChanMuxCtrlPort     =  chanMuxCtrlDataPort,
     .Appdataport         =  NwAppDataPort

    };


    ret = Seos_NwStack_init(instance, &nw_inst_0, &nw_data_0);

    if(ret<0)  // is possible when proxy does not run with use_tap =1 param. Just print and exit
    {
        Debug_LOG_INFO("Network Stack Init() Failed as Client...Exiting NwStack\n");
    }
    return 0;
}

