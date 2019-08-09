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
#include "seos_socket.h"

int run()
{
    Debug_LOG_INFO("starting network stack as Server...\n");
    int ret;

     nw_camkes_signal_glue  nw_signal_1 =
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

     nw_ports_glue nw_data_1 =
    {
            .ChanMuxDataPort = chanMuxDataPort_2,
            .ChanMuxCtrlPort = chanMuxCtrlDataPort_2,
            .Appdataport     = NwAppDataPort_2

    };

      Seos_nw_camkes_info nw_camkes_1 =
      {
          &nw_signal_1,
          &nw_data_1,
          SEOS_NWSTACK_AS_SERVER
      };


    // should never return as this starts pico_stack_tick().

    ret = Seos_NwStack_init(&nw_camkes_1);

    if(ret<0)  // is possible when proxy does not run with use_tap =1 param. Just print and exit
    {
        Debug_LOG_INFO("Network Stack Init()-Sever Failed...Exiting NwStack-2\n");
    }
    return 0;
}

