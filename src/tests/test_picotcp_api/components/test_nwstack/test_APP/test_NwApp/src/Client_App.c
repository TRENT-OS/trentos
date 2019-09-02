/*
 *  SEOS Network Stack CAmkES App as client
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */

#include <string.h>
#include "stdint.h"
#include "LibDebug/Debug.h"
#include "seos_err.h"

#include "seos_nw_api.h"

extern seos_err_t Seos_NwAPP_RT(Seos_nw_context ctx);

/*
    This example demonstrates reading of a web page example.com using Nw Stack API.
    Currently only a single socket is supported per stack instance.
    i.e. no multitasking is supported as of now.

*/

#define HTTP_PORT 80

int run()
{
    char buffer[4096];

    seos_nw_client_struct cli_socket =
    {
        .domain = SEOS_AF_INET,
        .type   = SEOS_SOCK_STREAM,
        .name   = "93.184.216.34",
        .port   = HTTP_PORT
    };

    seos_socket_handle_t handle;

    Seos_NwAPP_RT(NULL);   // Must be actullay called by SEOS Runtime

    Debug_LOG_INFO("Starting App as Client...\n");


    /* This creates a socket API and gives an handle which can be used
       for further communication. */
    seos_err_t err = Seos_client_socket_create(NULL, &cli_socket, &handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error creating App socket...error:%d\n", err);
        return -1;
    }


    const char* request =
        "GET / HTTP/1.0\r\nHost: example.com\r\nConnection: close\r\n\r\n";

    const size_t len_request = strlen(request);
    size_t len = len_request;


    /* Send the request to the host */
    size_t offs = 0;
    do
    {
        const size_t lenRemaining = len_request - offs;
        size_t len_io = lenRemaining;

        err = Seos_socket_write(handle, &request[offs], &len_io);

        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("Client socket write failure. %s, error:%d\n", __FUNCTION__,
                              err);
            Seos_socket_close(handle);
            return -1;  /* return write error reason */
        }

        if (len_io > lenRemaining)
        {
            /* fatal error, this must not happen. API broken*/
            Debug_ASSERT(false);
        }

        offs += len_io;

    }
    while (offs < len_request);


    Debug_LOG_INFO("Client socket read Webpage start. %s\n", __FUNCTION__);




    /*
    As of now the nw stack behavior is as below:
    Keep reading data until you receive one of the return values:
     a. err = SEOS_ERROR_CONNECTION_CLOSED indicating end of data read
              and connection close
     b. err = SEOS_ERROR_GENERIC  due to error in read
     c. err = SEOS_SUCCESS and length = 0 indicating no data to read but there is still
              connection
     d. err = SEOS_SUCCESS and length >0 , valid data

    Take appropriate actions based on the return value rxd.



    Only a single socket is supported and no multithreading !!!
    Once a webpage is read , display the contents.
    */

    int flag = true;

    do
    {
        len = sizeof(buffer);

        /* Keep calling read until we receive CONNECTION_CLOSED from the stack */
        memset(buffer, 0, sizeof(buffer));

        seos_err_t err = Seos_socket_read(handle, buffer, &len);

        switch (err)
        {

        /* This means end of read or nothing further to read as socket was closed */
        case  SEOS_ERROR_CONNECTION_CLOSED:

            flag = false;    /* terminate loop and close handle*/
            Debug_LOG_INFO(" Client app read completed..\n");
            break;

        /* Success . continue further reading */
        case  SEOS_SUCCESS:

            if (len > 0)
            {
                Debug_LOG_INFO("Web page read length %d and data:\n %s\n", len, buffer);
            }
            continue ;

        /* Error case, break and close the handle */
        default:
            Debug_LOG_WARNING(" Read failure. Closing socket!!!.\n");
            flag = false;    /* terminate loop and close handle */
            break;
        }// end of switch


    }
    while (flag);

    /* Close the socket communication */
    err = Seos_socket_close(handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Client socket close failure. %s, error :%d\n", __FUNCTION__,
                          err);
        return -1;
    }

    return 0;
}
