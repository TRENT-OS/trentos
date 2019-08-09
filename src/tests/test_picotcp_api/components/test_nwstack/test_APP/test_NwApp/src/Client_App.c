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
    int len;
    Seos_NwAPP_RT(NULL);   // Must be actullay called by SEOS Runtime

    seos_socket_handle_t socket;

    Debug_LOG_INFO("Starting App as Client...\n");

    char buffer[4096];


    seos_err_t err = Seos_socket_create(NULL, AF_INET, SOCK_STREAM,
                                        &socket); // SOCK_DGRAM

    if (err < 0)
    {
        Debug_LOG_INFO("Client App socket creation failure. %s\n", __FUNCTION__);
        Debug_ASSERT(0);
    }

    Debug_LOG_INFO("Client socket connect start. %s, socket=%d\n", __FUNCTION__,
                   socket);

    if (Seos_socket_connect(socket, "93.184.216.34",
                            HTTP_PORT) < 0) // connect example.com
    {
        Debug_LOG_INFO("Client socket connect failure. %s\n", __FUNCTION__);
        Debug_ASSERT(0);
    }
    const char* request =
        "GET / HTTP/1.0\r\nHost: example.com\r\nConnection: close\r\n\r\n";

    memcpy(buffer, request, strlen(request));
    len = strlen(request);

    do
    {
        seos_err_t err = Seos_socket_write(socket, buffer, &len);
        if (err < 0)
        {
            Debug_LOG_INFO("Client socket write failure. %s\n", __FUNCTION__);
            Debug_ASSERT(0);
        }
    }
    while (len < strlen(request));


    Debug_LOG_INFO("Client socket read Webpage start. %s\n", __FUNCTION__);

    len = 4096;

    while (1)
    {

        bzero(buffer, 4096);
        seos_err_t err = Seos_socket_read(socket, buffer, &len);

        if (err < 0)
        {
            Debug_LOG_INFO("Client socket read failure. %s\n", __FUNCTION__);
            Debug_ASSERT(0);
        }

        if (len == 0)
        {
            Debug_LOG_INFO(" Client app read completed..\n");
            break;
        }

        Debug_LOG_INFO("Buffer read length %d\n", (int)strlen(buffer));
        Debug_LOG_INFO("%s\n", buffer);
    }

    if (Seos_socket_close(socket) < 0)
    {
        Debug_LOG_INFO("Client socket close failure. %s\n", __FUNCTION__);
        Debug_ASSERT(0);
    }

    return 0;
}
