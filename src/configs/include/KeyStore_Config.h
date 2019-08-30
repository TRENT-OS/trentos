/**
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
#pragma once

#define KEY_INT_PROPERTY_LEN    4   // sizeof(size_t)

#define MAX_KEY_LEN             256 // maximum length of the raw key in bytes
#define MAX_KEY_NAME_LEN        16  // maximum length of the key name (including the null char)
#define DELIMITER_STRING        "," /* delimiter used for separating the parameters inside a file
                                       when saving a key (i.e. keyLen, keyBytes, algorithm, flags)*/

#define RNG_SEED                "9f19a9b95fea4d3419f39697ed54fd32"