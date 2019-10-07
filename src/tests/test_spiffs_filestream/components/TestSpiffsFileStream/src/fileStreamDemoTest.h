/**
 * @addtogroup Test_Spiffs_FileStream
 * @{
 *
 * @file fileStreamDemoTest.h
 *
 * @brief Collection of spiffs filestream demo tests
 *
 * Copyright (C) 2019, Hensoldt Cyber GmbH
 */

/**
 * @weakgroup Test_Spiffs_FileStream_DemoTests
 * @{
 * 
 * @brief A demo that tests a typical use case of the spiffs filestream
 *
 * @test \b Test_Spiffs_FileStream_test_case_04     \n  1)  Create a filestream with 
 *                                                  \n  2)  Write dummy string to it
 *                                                  \n  3)  Position the pointer to the beginning of the file
 *                                                  \n  4)  Check that the available space on the file and that it is
 *                                                          equal to the size of the write buffer (the pointer is at the
 *                                                          beginning of the file and it's size is sizeof(writeBuf))
 *                                                  \n  5)  Reopen the file as read-only
 *                                                  \n  6)  Read the entire written data
 *                                                  \n  7)  Verify that the read string matches the written one
 *                                                  \n  8)  Position the pointer to the beginning of the file
 *                                                  \n  9)  Read just the first part of the file (up to the first delimiter)
 *                                                  \n  10) Verify that the read string matches the first part of 
 *                                                          the original one
 *                                                  \n  11) Delete the opened file
 * 
 * @}
 */
seos_err_t spiffsFileStreamDemoTest_run(FileStreamFactory* fsFactory);

///@}
