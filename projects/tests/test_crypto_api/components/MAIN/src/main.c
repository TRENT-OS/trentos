#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>
#include <string.h>

#include "seos/SeosCryptoClient.h"
#include "seos/SeosCryptoDigest.h"

static void
testRNG ( SeosCryptoClient *client )
{
    seos_err_t err = SEOS_ERROR_GENERIC;
    void const *data;

    for ( int i = 0; i < 3; i++ )
    {
        err = SeosCryptoClient_getRandomData ( client, 0, &data, 16 );
        Debug_ASSERT ( err == SEOS_SUCCESS );

        Debug_PRINTF ( "Printing random bytes..." );
        for ( unsigned j = 0; j < 16; j++ )
        {
            Debug_PRINTF ( " 0x%02x", ( ( char * ) data ) [j] );
        }
        Debug_PRINTF ( "\n" );
    }
}

static void
testDigestMD5 ( SeosCryptoClient *client )
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    Debug_PRINTFLN ( "%s", "Testing Digest functions.." );

    err = SeosCryptoClient_digestInit ( client,
                                        SeosCryptoDigest_Algorithm_MD5,
                                        NULL,
                                        0 );
    Debug_ASSERT_PRINTFLN ( err == SEOS_SUCCESS, "err %d", err );

    const char *string = "0123456789";
    char *digest = NULL;
    size_t digestSize = 0;
    err = SeosCryptoClient_digestFinalize ( client,
                                            string,
                                            strlen ( string ),
                                            &digest,
                                            &digestSize );
    Debug_ASSERT ( err == SEOS_SUCCESS );

    Debug_PRINTF ( "Printing MD5 digest..." );
    for ( unsigned j = 0; j < digestSize; j++ )
    {
        Debug_PRINTF ( " 0x%02x", digest[j] );
    }
    Debug_PRINTF ( "\n" );
}

int run()
{
    SeosCryptoClient client;
    SeosCryptoRpc_Handle rpcHandle = NULL;
    seos_err_t err = SEOS_ERROR_GENERIC;

    err = Crypto_getRpcHandle ( &rpcHandle );
    Debug_ASSERT ( err == SEOS_SUCCESS );
    Debug_LOG_INFO ( "%s: got rpc object %p from server", __func__, rpcHandle );

    err = SeosCryptoClient_ctor ( &client, rpcHandle, cryptoClientDataport );
    Debug_ASSERT ( err == SEOS_SUCCESS );

    testRNG ( &client );
    testDigestMD5 ( &client );

    return 0;
}
