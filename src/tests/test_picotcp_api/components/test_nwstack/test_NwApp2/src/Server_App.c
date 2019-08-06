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
    Seos_NwAPP_RT(NULL);    // Must be actullay called by SEOS Runtime

    seos_socket_handle_t socket;
    seos_socket_handle_t client_socket;

    printf("starting Server app...\n");

    char buffer[4096];

   // char pszRequest[100]={0};


    seos_err_t err= Seos_socket_create(NULL,AF_INET,SOCK_STREAM, &socket);  // SOCK_DGRAM;

    if(err<0)
    {
        Debug_LOG_INFO("Server socket creation failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }

    Debug_LOG_INFO("Seos socket Accept start. %s, socket=%d\n",__FUNCTION__,socket);
    uint16_t listen_port = 5555;

    if(Seos_socket_bind(socket,listen_port) < 0)  // connect google.com , 172.217.22.46)
    {
        Debug_LOG_INFO("Server socket bind failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }

    if(Seos_socket_listen(socket,1) < 0)
    {
       Debug_LOG_INFO("Server socket listen failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }

    uint16_t port = 0;
    if(Seos_socket_accept(socket,&client_socket, port) < 0)
    {
       Debug_LOG_INFO("Server socket accept failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }
    Debug_LOG_INFO("Launching Server echo server\n");

    int n ;
    while(1)
   {

       bzero(buffer,4096);
       n =  Seos_socket_read(client_socket, buffer, 4096);

       if(n<0)
       {
         Debug_LOG_INFO(" Server socket read failure. %s\n",__FUNCTION__);
         Debug_ASSERT(0);
       }

       if(n==0)
       {
         break;
       }

       Debug_LOG_INFO("%d\n",(int)strlen(buffer));
       Debug_LOG_INFO("%s\n",buffer);

       Debug_LOG_INFO("Server write back echo data %d\n",(int)strlen(buffer));


       if(Seos_socket_write(client_socket,buffer, n) <= 0)
       {
           Debug_LOG_INFO("App-2 error write back echo data %d\n",(int)strlen(buffer));
           break;
       }
       else
       {
           break;
       }

   }

    if(Seos_socket_close(socket) <0)
    {
       Debug_LOG_INFO("NwApp 2 socket close failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }


   return 0;
}
