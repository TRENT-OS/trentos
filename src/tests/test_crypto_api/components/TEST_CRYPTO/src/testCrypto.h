/**
 * @addtogroup SEOS_Tests
 * @{
 *
 * @file testCrypto.h
 *
 * @brief Collection of tests for the functinalities of the crypto API
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#pragma once

#include "SeosCryptoCtx.h"

void testRNG(SeosCryptoCtx* cryptoCtx);

void testDigestMD5(SeosCryptoCtx* cryptoCtx);

void testDigestSHA256(SeosCryptoCtx* cryptoCtx);

void testCipherAES(SeosCryptoCtx* cryptoCtx);

///@}