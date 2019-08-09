/*
 *  SEOS Network Stack CAmkES App as Server
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#include <string.h>
#include "stdint.h"
#include "LibDebug/Debug.h"
#include "seos_socket.h"
#include "seos_err.h"

#include "seos_nw_api.h"

extern seos_err_t Seos_NwAPP_RT(Seos_nw_context ctx);


#define APP2_PORT 88

int run()
{
    char buffer[4096];
    Seos_NwAPP_RT(NULL);    // Must be actullay called by SEOS Runtime


    seos_nw_server_struct  socket =
    {
        .domain = AF_INET,
        .type   = SOCK_STREAM,
        .listen_port = 5555,
        .backlog = 1,
    };



    printf("starting Server app...\n");

    Seos_server_socket_create(NULL, &socket);

    Debug_LOG_INFO("Launching Server echo server\n");

    int n = 4096;
    while (1)
    {

        bzero(buffer, 4096);
        seos_err_t err =  Seos_socket_read(socket.client_handle, buffer, &n);

        if (err < 0)
        {
            Debug_LOG_INFO(" Server socket read failure. %s\n", __FUNCTION__);
            Debug_ASSERT(0);
        }

        if (n == 0)
        {
            break;
        }

        Debug_LOG_INFO("%d\n", (int)strlen(buffer));
        Debug_LOG_INFO("%s\n", buffer);

        Debug_LOG_INFO("Server write back echo data %d\n", (int)strlen(buffer));


        if (Seos_socket_write(socket.client_handle, buffer, &n) < 0)
        {
            Debug_LOG_INFO("App-2 error write back echo data %d\n", (int)strlen(buffer));
            break;
        }
        else
        {
            break;
        }

    }

    if (Seos_socket_close(socket.server_handle) < 0)
    {
        Debug_LOG_INFO("NwApp 2 socket close failure. %s\n", __FUNCTION__);
        Debug_ASSERT(0);
    }


    return 0;
}
