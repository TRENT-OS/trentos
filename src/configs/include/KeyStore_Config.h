/**
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */
#pragma once

#define KEY_INT_PROPERTY_LEN    4   /* Used to initialize the buffers for serialization of the size_t type
                                    key properties - it represents the number of bytes that size_t type
                                    takes up in memory */

#define MAX_KEY_LEN             256 /* Maximum length of the raw key in bytes */
#define MAX_KEY_NAME_LEN        16  /* Maximum length of the key name (including the null char) */
#define DELIMITER_STRING        "," /* Delimiter used for separating the serialized key parameters inside 
                                    a file when saving a key (i.e. keyLen, keyBytes, algorithm, flags) */