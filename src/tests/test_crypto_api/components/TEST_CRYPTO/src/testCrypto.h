/**
 * @addtogroup CryptoApi_Tests
 * @{
 *
 * @file testCrypto.h
 *
 * @brief Collection of tests for the functinalities of the crypto API
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

/**
 * @weakgroup Crypto_test_cases
 * @{
 *
 * @brief Collection of crypto api tests
 *
 *
 * @test testRNG                Test case for the random number generator
 *
 * @test testDigestMD5          Test case for the MD5 hash
 *
 * @test testDigestSHA256       Test case for the SHA256 hash
 *
 * @test testCipherAES          Test case for the AES cipher
 *
 * @}
 */

#pragma once

#include "SeosCryptoCtx.h"

void testRNG(SeosCryptoCtx* cryptoCtx);

void testDigestMD5(SeosCryptoCtx* cryptoCtx);

void testDigestSHA256(SeosCryptoCtx* cryptoCtx);

void testCipherAES(SeosCryptoCtx* cryptoCtx);

///@}