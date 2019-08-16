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


    seos_nw_server_struct  srv_socket =
    {
        .domain = AF_INET,
        .type   = SOCK_STREAM,
        .listen_port = 5555,
        .backlog = 1,
    };

    seos_socket_handle_t  seos_socket_handle ; // Gets filled when accept is called
    // Gets filled when server socket create is called
    seos_nw_server_handle_t  seos_nw_server_handle ;



    printf("starting Server app...\n");

    seos_err_t err = Seos_server_socket_create(NULL, &srv_socket,
                                               &seos_nw_server_handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error creating Server socket. Exiting error:%d !!!\n", err);
        return err;
    }

    Debug_LOG_INFO("Launching Server echo server\n");


    err = Seos_socket_accept(seos_nw_server_handle, &seos_socket_handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error accepting incoming socket connection. Exiting, error : %d !!!\n",
                          err);
        return -1;
    }

    int n = 4096;
    while (1)
    {

        memset(buffer, 0, 4096);
        err =  Seos_socket_read(seos_socket_handle, buffer, &n);

        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING(" Server socket read failure. %s, error: %d \n", __FUNCTION__,
                              err);
            Seos_socket_close(seos_socket_handle);
            return err;
        }

        if (n == 0)
        {
            break;
        }

        Debug_LOG_INFO("buffer read len = %d\n", n);
        Debug_LOG_INFO("%s\n", buffer);

        Debug_LOG_INFO("Server write back echo data %d\n", n);


        err = Seos_socket_write(seos_socket_handle, buffer, &n);

        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("App-2 error write back echo data %d, error code:%d\n", n,
                              err);
            Seos_socket_close(seos_socket_handle);
            return err;
        }

        break;
    }


    err = Seos_socket_close(seos_socket_handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("NwApp 2 socket client handle close failure. %s error code:%d\n",
                          __FUNCTION__, err);
        return err;
    }


    err = Seos_server_socket_close(seos_nw_server_handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("NwApp 2 socket server handle close failure. %s, error code: %d\n",
                          __FUNCTION__, err);
        return err;
    }


    return 0;
}
