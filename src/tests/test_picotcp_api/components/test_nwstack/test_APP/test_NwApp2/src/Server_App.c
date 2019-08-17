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

/*
    This example demonstrates a server with an incoming connection. Reads incoming data after
    connection is established. Writes or echoes the received data back to the client
    and then terminates the connection.
    Currently only a single socket is supported per stack instance.
    i.e. no multitasking is supported as of now.
*/


extern seos_err_t Seos_NwAPP_RT(Seos_nw_context ctx);


int run()
{
    char buffer[4096];
    Seos_NwAPP_RT(NULL);    /* Must be actullay called by SEOS Runtime */


    seos_nw_server_struct  srv_socket =
    {
        .domain = AF_INET,
        .type   = SOCK_STREAM,
        .listen_port = 5555,
        .backlog = 1,
    };

    /* Gets filled when accept is called */
    seos_socket_handle_t  seos_socket_handle ;
    /* Gets filled when server socket create is called */
    seos_nw_server_handle_t  seos_nw_server_handle ;



    printf("starting Server app...\n");

    seos_err_t err = Seos_server_socket_create(NULL, &srv_socket,
                                               &seos_nw_server_handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error creating Server socket. Exiting error:%d !!!\n", err);
        return -1;
    }

    Debug_LOG_INFO("Launching Server echo server\n");


    err = Seos_socket_accept(seos_nw_server_handle, &seos_socket_handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error accepting incoming socket connection. Exiting, error : %d !!!\n",
                          err);
        return -1;
    }

    /*
        As of now the nw stack behavior is as below:
        Keep reading data until you receive one of the return values:
         a. err = SEOS_ERROR_CONNECTION_CLOSED and length = 0 indicating end of data read
                  and connection close
         b. err = SEOS_ERROR_GENERIC  due to error in read
         c. err = SEOS_SUCCESS and length = 0 indicating no data to read but there is still
                  connection
         d. err = SEOS_SUCCESS and length >0 , valid data

        Take appropriate actions based on the return value rxd.


        Only a single socket is supported and no multithreading !!!
        Once we accept an incoming connection, start reading data from the client and echo back
        the data rxd.
    */

    while (1)
    {

        int n = sizeof(buffer);

        memset(buffer, 0, sizeof(buffer));

        /* Keep calling read until we receive CONNECTION_CLOSED from the stack */

        err =  Seos_socket_read(seos_socket_handle, buffer, &n);

        /* This is a case of read failure. We must perform a clean exit now */

        if (err == SEOS_ERROR_GENERIC)
        {
            Debug_LOG_WARNING(" Server socket read failure. %s, error: %d \n", __FUNCTION__,
                              err);
            Seos_socket_close(seos_socket_handle);
            Seos_server_socket_close(seos_nw_server_handle);
            return -1;
        }

        /*
        If there is nothing left to read and we receive CONNECTION_CLOSED
        from stack, we break and and then exit gracefully.

        */

        if ((err == SEOS_ERROR_CONNECTION_CLOSED) && (0 == n))
        {

            Debug_LOG_INFO(" Closing server socket communication !!\n ");
            break;
        }

        /* This indicates nothing to read , continue.*/
        if ((err == SEOS_SUCCESS) && (n == 0))
        {
            continue;
        }
        Debug_LOG_INFO(" rx data : %s\n", buffer);
        Debug_LOG_INFO(" Server echo back rx data !!..\n");

        err = Seos_socket_write(seos_socket_handle, buffer, &n);

        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("Server socket: error write back echo data %d, error code:%d\n",
                              n,
                              err);
            Seos_socket_close(seos_socket_handle);
            Seos_server_socket_close(seos_nw_server_handle);
            return -1;
        }

    }


    err = Seos_socket_close(seos_socket_handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Server socket: client handle close failure. %s error code:%d\n",
                          __FUNCTION__, err);
        return -1;
    }


    err = Seos_server_socket_close(seos_nw_server_handle);
    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Server socket: server handle close failure. %s, error code: %d\n",
                          __FUNCTION__, err);
        return -1;
    }


    return 0;
}
