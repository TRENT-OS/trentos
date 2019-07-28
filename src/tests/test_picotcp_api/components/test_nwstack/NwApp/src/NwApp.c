//#include "NwStack.h"
#include <camkes.h>
#include <string.h>
#include "LibDebug/Debug.h"
#include "seos_socket.h"
//#include "SeosNwStack.h"



#define HTTP_PORT 80

int run()
{

    Debug_LOG_INFO("Waiting in Nw App....\n");
    c_initdone_wait();                    // wait until Nw stack is initialized
    Debug_LOG_INFO("Starting Nw App...\n");

    char buffer[4096];
    int w_size=0;



    int socket= NwStackIf_socket(AF_INET,SOCK_STREAM);  // SOCK_DGRAM

    if(socket < 0)
    {
        Debug_LOG_INFO("NwApp socket creation failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }
    Debug_LOG_INFO("NwApp socket connect start. %s, socket=%d\n",__FUNCTION__,socket);

    if(NwStackIf_connect("93.184.216.34",HTTP_PORT) < 0)  // connect example.com
    {
        Debug_LOG_INFO("NwApp socket connect failure. %s\n",__FUNCTION__);
        Debug_ASSERT(0);
    }
    const char * request = "GET / HTTP/1.0\r\nHost: example.com\r\nConnection: close\r\n\r\n";

    memcpy(NwAppDataPort, request, strlen(request));

    while(w_size < strlen(request))
    {
         w_size = NwStackIf_write(strlen(request));
         if(w_size <0)
         {
             Debug_LOG_INFO("NwApp socket write failure. %s\n",__FUNCTION__);
             Debug_ASSERT(0);
         }
    }


    Debug_LOG_INFO("NwApp socket read start. %s\n",__FUNCTION__);

    while(1)
   {

     bzero(buffer,4096);
     int n = NwStackIf_read(4096);

     if(n<0)
     {
         Debug_LOG_INFO("NwApp socket read failure. %s\n",__FUNCTION__);
         Debug_ASSERT(0);
     }

     if(n==0)
     {
         break;
     }

     memcpy(buffer, NwAppDataPort, n);
     Debug_LOG_INFO("%d\n",(int)strlen(buffer));
     Debug_LOG_INFO("%s\n",buffer);
   }

    if(NwStackIf_close() <0)
    {
       Debug_LOG_INFO("NwApp socket close failure. %s\n",__FUNCTION__);
       Debug_ASSERT(0);
    }

    return 0;
}
