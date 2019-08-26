/**
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */

#include "testCrypto.h"
#include "SeosCryptoApi.h"

void testRNG(SeosCryptoCtx* cryptoCtx)
{
    seos_err_t err = SEOS_ERROR_GENERIC;
    char data[16];

    for (int i = 0; i < 3; i++)
    {
        err = SeosCryptoApi_getRandomData(cryptoCtx,
                                          0, // flags
                                          i > 0 ? data : NULL, sizeof(data), // salt buffer
                                          data, sizeof(data));
        Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

        Debug_PRINTF("Printing random bytes...");
        for (unsigned j = 0; j < sizeof(data); j++)
        {
            Debug_PRINTF(" 0x%02x", data[j]);
        }
        Debug_PRINTF("\n");
    }
}

void testDigestMD5(SeosCryptoCtx* cryptoCtx)
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    SeosCrypto_DigestHandle handle;

    err = SeosCryptoApi_digestInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoDigest_Algorithm_MD5,
                                   NULL,
                                   0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    const char* data        = "0123456789";
    const size_t dataLen    = strlen(data);
    char buff[SeosCryptoDigest_SIZE_MD5];
    void*  digest = buff;
    size_t digestSize = sizeof(buff);
    err = SeosCryptoApi_digestFinalize(cryptoCtx,
                                       handle,
                                       data,
                                       dataLen,
                                       &digest,
                                       &digestSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    Debug_PRINTF("Printing MD5 digest...");
    for (unsigned j = 0; j < digestSize; j++)
    {
        Debug_PRINTF(" 0x%02x", buff[j]);
    }
    Debug_PRINTF("\n");

    err = SeosCryptoApi_digestClose(cryptoCtx, handle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
}

void testDigestSHA256(SeosCryptoCtx* cryptoCtx)
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    SeosCrypto_DigestHandle handle;

    err = SeosCryptoApi_digestInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoDigest_Algorithm_SHA256,
                                   NULL,
                                   0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    const char* data = "0123456789";
    size_t dataLen = strlen(data);

    err = SeosCryptoApi_digestUpdate(cryptoCtx,
                                     handle,
                                     data,
                                     dataLen);

    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    void*   digest      = NULL;
    size_t  digestSize  = 0;
    err = SeosCryptoApi_digestFinalize(cryptoCtx,
                                       handle,
                                       NULL, 0,
                                       &digest, &digestSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    Debug_PRINTF("Printing SHA256 digest...");
    for (unsigned j = 0; j < digestSize; j++)
    {
        Debug_PRINTF(" 0x%02x", ((char*) digest)[j]);
    }
    Debug_PRINTF("\n");

    err = SeosCryptoApi_digestClose(cryptoCtx, handle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
}

void testCipherAES(SeosCryptoCtx* cryptoCtx)
{
    seos_err_t err = SEOS_ERROR_GENERIC;

    SeosCrypto_KeyHandle keyHandle;
    SeosCrypto_CipherHandle handle;

    const char*  data   = "0123456789ABCDEF";
    size_t dataLen      = strlen(data);

    char buff[16];
    void* output = buff;
    size_t outputSize = sizeof(buff);

    err = SeosCryptoApi_keyImport(cryptoCtx,
                                  &keyHandle,
                                  SeosCryptoCipher_Algorithm_AES_ECB_ENC,
                                  BitMap_MASK_OF_BIT(SeosCryptoKey_Flags_IS_ALGO_CIPHER),
                                  "0123456789ABCDEF",
                                  128);
    Debug_ASSERT_PRINTFLN(SEOS_SUCCESS == err, "err %d", err);

    err = SeosCryptoApi_cipherInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoCipher_Algorithm_AES_ECB_ENC,
                                   keyHandle,
                                   NULL, 0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoApi_cipherUpdate(cryptoCtx,
                                     handle,
                                     data,
                                     dataLen,
                                     &output,
                                     &outputSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
    Debug_ASSERT(outputSize == dataLen);

    Debug_PRINTF("Printing AES ciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", ((char*) output)[j]);
    }
    Debug_PRINTF("\n");

    err = SeosCryptoApi_cipherClose(cryptoCtx, handle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoApi_cipherInit(cryptoCtx,
                                   &handle,
                                   SeosCryptoCipher_Algorithm_AES_ECB_DEC,
                                   keyHandle,
                                   NULL, 0);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    data    = output;
    dataLen = outputSize;
    output  = NULL;

    err = SeosCryptoApi_cipherUpdate(cryptoCtx,
                                     handle,
                                     data,
                                     dataLen,
                                     &output,
                                     &outputSize);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    Debug_PRINTF("Printing AES deciphered data ...");
    for (unsigned j = 0; j < outputSize; j++)
    {
        Debug_PRINTF(" 0x%02x", ((char*) output)[j]);
    }
    Debug_PRINTF("\n");

    err = SeosCryptoApi_cipherClose(cryptoCtx, handle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);

    err = SeosCryptoApi_keyClose(cryptoCtx, keyHandle);
    Debug_ASSERT_PRINTFLN(err == SEOS_SUCCESS, "err %d", err);
}