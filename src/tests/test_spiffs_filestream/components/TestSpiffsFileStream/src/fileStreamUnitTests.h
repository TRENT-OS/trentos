/**
 * @addtogroup Test_Spiffs_FileStream
 * @{
 *
 * @file fileStreamUnitTests.h
 *
 * @brief Collection of spiffs filestream unit tests
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

/**
 * @weakgroup Test_Spiffs_FileStream_UnitTests
 * @{
 *
 * @brief Unit tests for the create filestream function
 *
 * @test \b Test_Spiffs_FileStream_test_case_01     \n <b> 1) Positive cases: </b>
 *                                                  \n          Create a filestream for every open mode (r, w, r/w, a)
 *                                                  \n <b> 2) Negative cases (check for proper error codes): </b>
 *                                                  \n          Create a filestream with a too long name
 *                                                  \n          Create a filestream with an empty string for a name
 *                                                  \n          Create a filestream with NULL passed for a name
 *                                                  \n          Try to create a filestream with mode = r, when it does not previously exist
 * @}
 */
seos_err_t test_createFileStream(FileStreamFactory* fsFactory);
/**
 * @weakgroup Test_Spiffs_FileStream_UnitTests
 * @{
 *
 * @brief Unit tests for the write filestream function
 *
 * @test \b Test_Spiffs_FileStream_test_case_02     \n <b> 1) Positive cases: </b>
 *                                                  \n          Write an example string to a filestream opened in w mode
 *                                                  \n <b> 2) Negative cases (check for proper error codes): </b>
 *                                                  \n          Try to pass NULL as a write buffer
 *                                                  \n          Try to pass 0 as the write length
 *                                                  \n          Try to pass NULL as a write buffer and 0 as the write length
 * @}
 */
seos_err_t test_writeFileStream(FileStreamFactory* fsFactory);
/**
 * @weakgroup Test_Spiffs_FileStream_UnitTests
 * @{
 *
 * @brief Unit tests for the read filestream function
 *
 * @test \b Test_Spiffs_FileStream_test_case_03     \n <b> 1) Positive cases: </b>
 *                                                  \n          Read an example string from a filestream opened in r
 *                                                              mode and verify that the read string matches the original one
 *                                                  \n <b> 2) Negative cases (check for proper error codes): </b>
 *                                                  \n          Try to pass NULL as a read buffer
 *                                                  \n          Try to pass 0 as the read length
 *                                                  \n          Try to pass NULL as a read buffer and 0 as the read length
 * @}
 */
seos_err_t test_readFileStream(FileStreamFactory* fsFactory);

///@}