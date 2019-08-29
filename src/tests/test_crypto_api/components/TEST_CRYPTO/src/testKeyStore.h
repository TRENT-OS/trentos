/**
 * @addtogroup CryptoApi_Tests
 * @{
 *
 * @file testKeyStore.h
 *
 * @brief collection of tests for the KeyStoreApi
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#pragma once

#include "SeosKeyStoreCtx.h"
#include "SeosCryptoCtx.h"

/**
 * @brief Generic test scenario for the key store which is customized
 * by passing different input parameters
 *
 * @param keyStoreCtx               handle to the keyStore, it can represent a local instance
 *                                  of the key store library, or a handle to the context which
 *                                  is created in a separate camkes component
 * @param cryptoCtx                 handle to the crypto library, it can represent a local instance
 *                                  of the library, or a handle to the context which is created in a
 *                                  separate camkes component
 * @param generateKey               a flag that determines whether the test will generatea new key
 *                                  and perform the test scenario with it or import a predefined key
 *
 * @return                          true => test scenario passed
 *                                  false => test scenario failed
 *
 * @test TestKeyStore_testCase_01   Generate/import a key (based on the generateKey parameter) and perform
 *                                  AES encryption/decryption on a known string to verify the key functionality
 *
 * @test TestKeyStore_testCase_02   Close the key and try to perform AES encryption/decryption with it and verify
 *                                  that the lower levels throw an error since the key handle is no longer valid
 *
 * @test TestKeyStore_testCase_03   Get the previously imported/generated key and perform AES encryption/decryption
 *                                  on a known string to verify the key functionality
 *
 * @test TestKeyStore_testCase_04   Delete the key and try to perform AES encryption/decryption with it and verify
 *                                  that the lower levels throw an error since the key handle is no longer valid
 *
 */
bool testKeyStore(SeosKeyStoreCtx* keyStoreCtx, SeosCryptoCtx* cryptoCtx,
                  bool generateKey);

///@}

