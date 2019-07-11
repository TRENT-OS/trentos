#include "LibDebug/Debug.h"

#include <stdio.h>
#include <camkes.h>
#include <string.h>

#include "seos/SeosCryptoClient.h"
#include "seos/SeosCryptoDigest.h"
#include "seos/SeosCryptoCipher.h"

static void
testRNG(SeosCryptoClient* client)
{
    seos_err_t err = SEOS_ERROR_GENERIC;
    void const* data;

    for (int i = 0; i < 3; i++)
    {
        err = SeosCryptoClient_getRandomData(client, 0, &data, 16);
        Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

        Debug_PRINTF("Printing random bytes...");
        for (unsigned j = 0; j < 16; j++)
        {
            Debug_PRINTF(" 0x%02x", (( char*) data) [j]);
        }
        Debug_PRINTF("\n");
    }
}

static void
testDigestMD5(SeosCryptoClient* client)
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    Debug_PRINTFLN("%s", "Testing Digest functions..");

    err = SeosCryptoClient_digestInit(client,
                                      SeosCryptoDigest_Algorithm_MD5,
                                      NULL,
                                      0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    const char* string = "0123456789";
    char* digest = NULL;
    size_t digestSize = 0;
    err = SeosCryptoClient_digestFinalize(client,
                                          string,
                                          strlen(string),
                                          &digest,
                                          &digestSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    Debug_PRINTF("Printing MD5 digest...");
    for (unsigned j = 0; j < digestSize; j++)
    {
        Debug_PRINTF(" 0x%02x", digest[j]);
    }
    Debug_PRINTF("\n");

    SeosCryptoClient_digestClose(client);
}

static void
testDigestSHA256(SeosCryptoClient* client)
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    err = SeosCryptoClient_digestInit(client,
                                      SeosCryptoDigest_Algorithm_SHA256,
                                      NULL,
                                      0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    const char* string = "0123456789";

    err = SeosCryptoClient_digestUpdate(client,
                                        string,
                                        strlen(string));

    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    char digest[SeosCryptoDigest_SIZE_SHA256];
    err = SeosCryptoClient_digestFinalizeNoData2(client,
                                                 digest,
                                                 sizeof(digest));
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    Debug_PRINTF("Printing SHA256 digest...");
    for (unsigned j = 0; j < sizeof(digest); j++)
    {
        Debug_PRINTF(" 0x%02x", digest[j]);
    }
    Debug_PRINTF("\n");
}

static void
testCipherAES(SeosCryptoClient* client)
{
    SeosCryptoCipher scCipher;
    seos_err_t err = SEOS_ERROR_GENERIC;

    SeosCryptoKey key;

    err = SeosCryptoKey_ctor(&key,
                             NULL,
                             SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                             BitMap_MASK_OF_BIT(SeosCryptoKey_Flags_IS_ALGO_CIPHER),
                             "0123456789ABCDEF",
                             128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    char    buffer[16];
    char const*   input = key.bytes;
    size_t  inputSize   = strlen(key.bytes);
    char*   output      = buffer;
    size_t  outputSize  = sizeof(buffer);

    Debug_PRINTFLN("%s", "Testing Cipher functions..");

    err = SeosCryptoCipher_ctor(&scCipher,
                                SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                                &key,
                                NULL,
                                NULL,
                                0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoCipher_update(&scCipher,
                                  input,
                                  inputSize,
                                  &output,
                                  &outputSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_ASSERT(outputSize == inputSize);

    Debug_PRINTF("Printing AES ciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", output[j]);
    }
    Debug_PRINTF("\n");

    SeosCryptoCipher_dtor(&scCipher);

    err = SeosCryptoCipher_ctor(&scCipher,
                                SeosCryptoCipher_Algorithm_AES_EBC_DEC,
                                &key,
                                NULL,
                                NULL,
                                0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    input   = output;
    output  = NULL;

    err = SeosCryptoCipher_update(&scCipher,
                                  input,
                                  inputSize,
                                  &output,
                                  &outputSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_ASSERT(outputSize == inputSize);

    Debug_PRINTF("Printing AES deciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", output[j]);
    }
    Debug_PRINTF("\n");

// Now with client -----------------------------

    input       = key.bytes;
    inputSize   = strlen(key.bytes);
    output      = buffer;
    outputSize  = sizeof(buffer);

    SeosCrypto_KeyHandle keyHandle;

    err = SeosCryptoRpc_keyCreate(client->rpcHandle,
                                  SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                                  BitMap_MASK_OF_BIT(SeosCryptoKey_Flags_IS_ALGO_CIPHER),
                                  128,
                                  &keyHandle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoClient_cipherInit(client,
                                      SeosCryptoCipher_Algorithm_AES_EBC_ENC,
                                      keyHandle,
                                      NULL,
                                      0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    err = SeosCryptoClient_cipherUpdate(client,
                                        input,
                                        inputSize,
                                        &output,
                                        &outputSize);

    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_ASSERT(outputSize == inputSize);

    Debug_PRINTF("Printing AES ciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", output[j]);
    }
    Debug_PRINTF("\n");

    SeosCryptoClient_cipherClose(client);

    err = SeosCryptoClient_cipherInit(client,
                                      SeosCryptoCipher_Algorithm_AES_EBC_DEC,
                                      keyHandle,
                                      NULL,
                                      0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    input   = output;
    output  = NULL;

    err = SeosCryptoClient_cipherUpdate(client,
                                        input,
                                        inputSize,
                                        &output,
                                        &outputSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_ASSERT(outputSize == inputSize);

    Debug_PRINTF("Printing AES deciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", output[j]);
    }
    Debug_PRINTF("\n");
}

int run()
{
    SeosCryptoClient client;
    SeosCryptoRpc_Handle rpcHandle = NULL;
    seos_err_t err = SEOS_ERROR_GENERIC;

    err = Crypto_getRpcHandle(&rpcHandle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_LOG_INFO("%s: got rpc object %p from server", __func__, rpcHandle);

    err = SeosCryptoClient_ctor(&client, rpcHandle, cryptoClientDataport);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    testRNG(&client);
    testDigestMD5(&client);
    testDigestSHA256(&client);
    testCipherAES(&client);

    return 0;
}
