/*
 * SEOS Network Stack API

 * @addtogroup SEOS
 * @{
 *
 * @file seos_nw_api.h
 *
 * @brief SEOS Network API

 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */
#pragma once


typedef struct _Seos_NW_Server_t
{
    int domain;                         //  can be AF_INET
    int type;                           //  can be SOCK_STREAM
    uint16_t listen_port;               // port to listen to e.g. 5555
    int backlog;                        // number of connections accepted (currently fixed =1)
    seos_socket_handle_t server_handle; // socket handle for server
    seos_socket_handle_t
    client_handle; // socket handle of incoming connection. This must be used for read/write

} seos_nw_server_struct;


typedef struct _Seos_NW_Client_t
{
    int domain;                     // can be AF_INET
    int type;                       // can be SOCK_STREAM
    const char* name;               // IP addr to connect to
    uint16_t port;                  // e.g. HTTP port 80
    seos_socket_handle_t handle;    // socket handle

} seos_nw_client_struct;


/**
 * @brief Creates a network socket. The socket gets created and can then be used for read or write data.
 *
 * @param Seos_nw_context ctx (required). Passed by App which is the run time context received by APP.
 *
 * @param domain (required). Domain is of type AF_INET or AF_INET6. As of now we work with AF_INET only.
 *
 * @param type (required).  Type is of type SOCK_STREAM or SOCK_DGRAM. As of now we work only with SOCK_STREAM.
 *
 * @return Socket handle which is an integer and this handle will be used for all further communications over network.
 *         As of now the handle is for a single App/Thread per stack.

   SEOS_ERROR in case if there was an error creating a socket.
 *
 */

seos_err_t
Seos_socket_create(Seos_nw_context ctx,
                   int domain,
                   int type,
                   seos_socket_handle_t* pHandle );

/**
 * @brief Closes a network socket. Once the close is done no further socket communication is possible.
 *
 * @param handle. Handle used to open/create socket.
 *
 * @return SEOS_SUCCESS or SEOS_ERROR
 *
 */

seos_err_t
Seos_socket_close(seos_socket_handle_t handle);

/**
 * @brief Connect a network socket. Connect to an IP addr and port
 *
 * @param handle. Used to create/open socket

 * @param name. Connect to an IP addresss, such as "192.168.82.45"

 * @param port. Port  number to connect to . e.g. 80
 *
 * @return SEOS_SUCCESS or SEOS_ERROR
 *
 */
seos_err_t
Seos_socket_connect(seos_socket_handle_t handle,
                    const char* name,
                    int port);

/**
 * @brief Write to a network socket. Write data to a socket after connecting. Copy the data to be written
 *        in the CamkES App dataport.
 *
 * @param handle. Used to create/open socket
 *
 * @param buf. Buffer containing data to be written

 * @param len. Length of data to write
 *
 * @return Actual Number of Bytes written  or SEOS_ERROR
 *
 */
seos_err_t
Seos_socket_write(seos_socket_handle_t handle,
                  void* buf,
                  int* len);

/**
 * @brief Bind to a network port. This is useful when the Network stack is working as Server.

 * @param handle. Used to create/open socket
 *
 * @param port. Port to which you want to bind.
 *
 * @return SEOS_SUCCESS or SEOS_ERROR
 */

seos_err_t
Seos_socket_bind(seos_socket_handle_t handle,
                 uint16_t port);

/**
 * @brief Listen for incoming connections. This is useful when the Network stack is working as Server.
 *
 * @param handle. Used to create/open socket

 * @param backlog. Indicates how many incoming connections is possible (for now it is only 1 connection)
 *
 * @return SEOS_SUCCESS or SEOS_ERROR
 */

seos_err_t
Seos_socket_listen(seos_socket_handle_t handle,
                   int backlog);

/**
 * @brief Accept for incoming connections. This is useful when the Network stack is working as Server.
 *
 * @param handle. Used to create/open socket

 * @param port. Indicates which port you want to accept from.
 *
 * @return SEOS_SUCCESS or SEOS_ERROR
 */

seos_err_t
Seos_socket_accept(seos_socket_handle_t handle,
                   seos_socket_handle_t* pClient,
                   uint16_t port);

/**
 * @brief Read data from  connected socket.

 * @param handle, Used to create/open socket

 * @param buf, Buffer to read data into
 *
 * @param len, Indicates how much data to read.
 *
 * @return Actual number of bytes read  or SEOS_ERROR
 */

seos_err_t
Seos_socket_read(seos_socket_handle_t handle,
                 void* buf,
                 int* len);



/**
 * @brief Create a socket and connect to it

 * @param Seos_nw_context ctx (required). Passed by App which is the run time context received by APP.

 * @param seos_nw_client_struct. Must be filled by app. Please see the struct for details
 *
 * @return  SEOS_SUCCESS or SEOS_ERROR
 */

seos_err_t
Seos_client_socket_create(Seos_nw_context ctx,
                          seos_nw_client_struct* pSocket);

/**
 * @brief Create a server socket and accept an incoming connection

 * @param Seos_nw_context ctx (required). Passed by App which is the run time context received by APP.

 * @param seos_nw_server_struct. Must be filled by app. Please see the struct for details
 *
 * @return  SEOS_SUCCESS or SEOS_ERROR
 */

seos_err_t
Seos_server_socket_create(Seos_nw_context ctx,
                          seos_nw_server_struct* pServer);

