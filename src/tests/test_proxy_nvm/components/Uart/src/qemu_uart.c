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
#include "uart.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define UART_CONTROL                 0x00
#define UART_MODEM_CONTROL           0x24
#define UART_MODEM_OFFSET            0x28
#define UART_CHANNEL_STS             0x2C
#define UART_TX_RX_FIFO              0x30

#define UART_REG(baseAddress, x) \
    ((volatile uint32_t *)((uintptr_t)(baseAddress) + (x)))

#define UART_CONTROL_RX_EN           (1U << 2)
#define UART_CONTROL_RX_DIS          (1U << 3)

#define UART_CONTROL_TX_EN           (1U << 4)
#define UART_CONTROL_TX_DIS          (1U << 5)

#define UART_CHANNEL_STS_TXEMPTY     (1U << 3)
#define UART_CHANNEL_STS_RXEMPTY     (1U << 1)

#define UART_MODEMCR_FCM             (1U << 5)

#include <camkes.h>

static unsigned
readReg(unsigned offset)
{
    return (unsigned int)(*UART_REG(uartRegBase, offset));
}

static void
writeReg(unsigned offset, unsigned value)
{
    *UART_REG(uartRegBase, offset) = value;
}

void
Uart_enable(void)
{
    uint32_t v = readReg(UART_CONTROL);

    /* Enable writing. */
    v |= UART_CONTROL_TX_EN;
    v &= ~UART_CONTROL_TX_DIS;

    /* Enable reading. */
    v |= UART_CONTROL_RX_EN;
    v &= ~UART_CONTROL_RX_DIS;

    writeReg(UART_CONTROL, v);
}

void
Uart_putChar(char byte)
{
    writeReg(UART_TX_RX_FIFO, byte);
    // Wait completion, fifo empty
    while ((readReg(UART_CHANNEL_STS) & UART_CHANNEL_STS_TXEMPTY) == 0);
}

char
Uart_getChar(void)
{
    // Wait a byte, fifo not empty
    while ((readReg(UART_CHANNEL_STS) & UART_CHANNEL_STS_RXEMPTY) != 0);
    char byte = readReg(UART_TX_RX_FIFO);

    return byte;
}
