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


    seos_err_t err = Seos_client_socket_create(NULL, &cli_socket, &handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Error creating App socket...error:%d\n", err);
        return err;
    }


    const char* request =
        "GET / HTTP/1.0\r\nHost: example.com\r\nConnection: close\r\n\r\n";

    int len;
    int len2;

    len2 = strlen(request);
    len = len2 ;

    memcpy(buffer, request, len);

    do
    {
        seos_err_t err = Seos_socket_write(handle, buffer, &len);
        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("Client socket write failure. %s, error:%d\n", __FUNCTION__,
                              err);
            Seos_socket_close(handle);
            return err;  /* return write error reason */

        }
    }
    while (len < len2);


    Debug_LOG_INFO("Client socket read Webpage start. %s\n", __FUNCTION__);

    len = 4096;

    while (1)
    {

        memset(buffer, 0, 4096);
        seos_err_t err = Seos_socket_read(handle, buffer, &len);

        if (err != SEOS_SUCCESS)
        {
            Debug_LOG_WARNING("Client socket read failure. %s, error:%d\n", __FUNCTION__,
                              err);
            Seos_socket_close(handle);
            return err;    /* return read error reason */

        }

        if (len == 0)   /* len=0 indicates a close event was called */
        {
            Debug_LOG_INFO(" Client app read completed..\n");
            break;
        }

        Debug_LOG_INFO("Buffer read length %d\n", len);
        Debug_LOG_INFO("%s\n", buffer);
    }

    err = Seos_socket_close(handle);

    if (err != SEOS_SUCCESS)
    {
        Debug_LOG_WARNING("Client socket close failure. %s, error :%d\n", __FUNCTION__,
                          err);
        return err;
    }

    return 0;
}
