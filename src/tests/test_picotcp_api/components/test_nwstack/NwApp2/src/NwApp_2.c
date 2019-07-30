#include <camkes.h>
#include <string.h>
#include "LibDebug/Debug.h"
#include "seos_socket.h"

#define APP2_PORT 88

int run()
{

    seos_nw_if_init();    // Nw Stack is initialised
    printf("starting App as Server...\n");

    char buffer[4096];

   // char pszRequest[100]={0};


    int socket= seos_nw_if_socket(AF_INET,SOCK_STREAM);  // SOCK_DGRAM;

    if(socket<0)
    {
        Debug_LOG_INFO("NwApp 2 socket creation failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }

    Debug_LOG_INFO("NwApp 2 socket Accept start. %s, socket=%d\n",__FUNCTION__,socket);
    uint16_t listen_port = 5555;

    if(seos_nw_if_bind(listen_port) < 0)  // connect google.com , 172.217.22.46)
    {
        Debug_LOG_INFO("NwApp 2 socket bind failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }

    if(seos_nw_if_listen(1) < 0)
    {
       Debug_LOG_INFO("NwApp 2 socket listen failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }

    uint16_t port = 0;
    if(seos_nw_if_accept(port) < 0)
    {
       Debug_LOG_INFO("NwApp 2 socket accept failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }
    Debug_LOG_INFO("Launching nwApp2 echo server\n");

    int n ;
    while(1)
   {

       bzero(buffer,4096);
       n =  seos_nw_if_read(4096);

       if(n<0)
       {
         Debug_LOG_INFO("App2  socket read failure. %s\n",__FUNCTION__);
         Debug_ASSERT(0);
       }

       if(n==0)
       {
         break;
       }

       memcpy(buffer, NwAppDataPort_2, n);

       Debug_LOG_INFO("%d\n",(int)strlen(buffer));
       Debug_LOG_INFO("%s\n",buffer);

       Debug_LOG_INFO("App2 write back echo data %d\n",(int)strlen(buffer));

       memcpy(NwAppDataPort_2, buffer,n);

       if(seos_nw_if_write(n) <= 0)
       {
           Debug_LOG_INFO("App-2 error write back echo data %d\n",(int)strlen(buffer));
           break;
       }
       else
       {
           break;
       }

   }

    if(seos_nw_if_close() <0)
    {
       Debug_LOG_INFO("NwApp 2 socket close failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }


   return 0;
}
