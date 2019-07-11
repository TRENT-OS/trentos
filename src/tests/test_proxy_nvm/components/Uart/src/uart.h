#pragma once

/*
    Note: this module represents functionality of the UART contained on the zynq7000 platform.
    It has been created to provide a communication channel between a "host" process in Linux
    and the seL4 in case the zynq7000 is simulated by QEMU.

    It has been found that this specific UART is not completely implemented in QEMU: especially
    hardware flow control is missing. Because of this only polling access has been implemented
    so far.

    Up to now this code has only been tested with QEMU. In theory it could also be used in
    case the seL4 is running on a real zynq7000 board. (In which case we would want to use
    and implement hardware flow control.)
*/

void Uart_enable();
void Uart_putChar(char byte);
char Uart_getChar();
