/*
 *  SEOS Network App CAmkES wrapper
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 *  The API's provided here must be used by the Nw Application to create, connect,
 *  write and read data over socket connections.
 *
 */
#include <camkes.h>
#include <string.h>
#include "LibDebug/Debug.h"
#include "seos_socket.h"


typedef struct _app_nw_ports_glue_t
{
    void *Appdataport;
 }app_ports_glue ;

 app_ports_glue app_port, *pAppPort ;



/* This must actually get called during Seos Run time.
 * It must initialise NW stack with Camkes gluebefore an APP main() is triggered.
 *
 */

seos_err_t Seos_NwAPP_RT(Seos_nw_context ctx )
{

    seos_network_init();  // Initialise NW stack.
    pAppPort = &app_port;

    #if defined(CLIENT_CONFIG)
    pAppPort->Appdataport = NwAppDataPort;
    #elif defined(SERVER_CONFIG)
    pAppPort->Appdataport = NwAppDataPort_2;
    #else
    #error "Data Port not specified !!!"
    #endif


    return SEOS_SUCCESS;

}

/*Seos_nw_context : Currently not used, but for future expansion to decide if we want to communicate to which
 *                  Nwstack instance (e.g 1 or 2). It is void * as of now
 *
 */

seos_err_t Seos_socket_create(Seos_nw_context ctx, int domain, int type, seos_socket_handle_t *pHandle )
{

    seos_err_t err = seos_socket_create(domain,type, pHandle);
    return err;

}


seos_err_t Seos_socket_close(seos_socket_handle_t handle)
{
    seos_err_t err = seos_socket_close(handle);
    return err;
}


seos_err_t Seos_socket_connect(seos_socket_handle_t handle, const char* name, int port)
{
    seos_err_t err =  seos_socket_connect(handle, name, port);
    return err;
}


int  Seos_socket_write(seos_socket_handle_t handle, void * buf, int len)
{
    memcpy(pAppPort->Appdataport, buf , len);
    int written = seos_socket_write(handle,len);
    return written;
}


seos_err_t  Seos_socket_bind(seos_socket_handle_t handle, uint16_t port)
{
    seos_err_t err = seos_socket_bind(handle, port);
    return err;

}

seos_err_t  Seos_socket_listen(seos_socket_handle_t handle, int backlog)
{
    seos_err_t err = seos_socket_listen(handle,1);
    return err;

}

seos_err_t  Seos_socket_accept(seos_socket_handle_t handle, seos_socket_handle_t *pClientHandle, uint16_t port)
{
    seos_err_t err = seos_socket_accept(handle, pClientHandle, port);
    return err;

}

int  Seos_socket_read(seos_socket_handle_t handle, void *buf, int len)
{
    int read = seos_socket_read(handle,len);
    memcpy(buf, pAppPort->Appdataport, read);
    return read;

}


/* App as client. Unify the API's into single API
 *
 *  App can call this API directly
 *
 *  TBD
 */

