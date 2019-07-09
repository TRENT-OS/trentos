/*
 *  Common functions used for initializing and using the ProxyNVM
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#pragma once

#include <stddef.h>

/* Exported functions ------------------------------------------------------- */

/**
*@brief Initializes the ChanMuxClient and the ProxyNVM and maps the passed channel to the defined dataport.
*/
int
ProxyNVMTest_init(unsigned int chan);

/**
 *
 * @brief Tests the ProxyNVM by writing and then reading a passed amount of data from the NVM.
 *
 * @param address the address in the NVM memory
 * @param length the length of data to be written and read from the NVM
 * @param testName the specific name of the test, which will be printed for debugging
 */
bool
ProxyNVMTest_run(size_t address, size_t length, const char* testName);

