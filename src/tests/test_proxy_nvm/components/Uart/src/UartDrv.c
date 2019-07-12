#include "uart.h"
#include <camkes.h>

void UartDrv__init()
{
    Uart_enable();
}

void UartDrv_write(size_t len)
{
    const char* buf = (const char*) inputDataPort;

    for (size_t i = 0; i < len; i++)
    {
        Uart_putChar(buf[i]);
    }
}

int run()
{
    while (true)
    {
        Output_takeByte(Uart_getChar());
    }
}
