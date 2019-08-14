/*
 *  SEOS UART header
 *
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 *
 */

#pragma once



void Uart_enable();
void Uart_putChar(char byte);
char Uart_getChar();
