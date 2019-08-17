/*
 *  SEOS Network Stack CAmkES App as client
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
        .domain = AF_INET,
        .type   = SOCK_STREAM,
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

    const int len_request = strlen(request);
    int len = len_request;

    memcpy(buffer, request, len);

    do
    {
        seos_err_t err = Seos_socket_write(handle, buffer, &len);
        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("Client socket write failure. %s, error:%d\n", __FUNCTION__,
                              err);
            Seos_socket_close(handle);
            return -1;  /* return write error reason */

        }
    }
    while (len < len_request);


    Debug_LOG_INFO("Client socket read Webpage start. %s\n", __FUNCTION__);

    len = sizeof(buffer);


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
    Once a webpage is read , display the contents.
    */


    while (1)
    {

        /* Keep calling read until we receive CONNECTION_CLOSED from the stack */
        memset(buffer, 0, sizeof(buffer));

        seos_err_t err = Seos_socket_read(handle, buffer, &len);


        /*
        First check if there is nothing left to read and we receive CONNECTION_CLOSED
        from stack. This means we can break now as it is the end of read.
        */

        if ((err == SEOS_ERROR_CONNECTION_CLOSED) && (0 == len))
        {
            Debug_LOG_INFO(" Client app read completed..\n");
            break;
        }

        /* This is a case of read failure. We must perform a clean exit now */
        if (err == SEOS_ERROR_GENERIC)
        {
            Debug_LOG_WARNING(" Read failure. Closing socket!!!.\n");
            Seos_socket_close(handle);
            return -1;
        }

        /* This indicates nothing to read , continue.*/
        if ((err == SEOS_SUCCESS) && (len == 0))
        {
            continue;
        }


        Debug_LOG_INFO("Buffer read length %d and data:\n %s\n", len, buffer);
    }

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
