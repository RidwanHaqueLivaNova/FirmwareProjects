// FRAM DRIVER + UART DEBUG OUTPUT

#include "io430.h"

#define FRAM_I2C_BASE_ADDRESS 0x50

static void chip_init(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    DCOCTL  = 0x60;
    BCSCTL1 = 0x87;
    BCSCTL2 = 0x00;
    BCSCTL3 = 0x05;
}

static void uart_init(void)
{
    UCA1CTL1 |= UCSWRST;

    UCA1CTL1 |= UCSSEL_2;

    UCA1BR0 = 104;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_1;

    P3SEL |= BIT6 | BIT7;

    UCA1CTL1 &= ~UCSWRST;
}

static void i2c_init(void)
{
    UCB0CTL1 |= UCSWRST;

    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;

    UCB0CTL1 = UCSWRST | UCSSEL_2;

    UCB0BR0 = 10;
    UCB0BR1 = 0;

    P3SEL |= BIT1 | BIT2;

    UCB0CTL1 &= ~UCSWRST;
}

void hardware_init(void)
{
    chip_init();
    uart_init();
    i2c_init();
}

void uart_send_char(char c)
{
    while (!(UC1IFG & UCA1TXIFG))
    {
    }

    UCA1TXBUF = c;
}

void uart_send_string(const char s[])
{
    unsigned int i;

    i = 0;

    while (s[i] != '\0')
    {
        uart_send_char(s[i]);
        i++;
    }
}

static void uart_send_hex_nibble(unsigned char value)
{
    value &= 0x0F;

    if (value < 10)
    {
        uart_send_char((char)('0' + value));
    }
    else
    {
        uart_send_char((char)('A' + value - 10));
    }
}

void uart_send_hex_byte(unsigned char value)
{
    uart_send_string("0x");
    uart_send_hex_nibble((unsigned char)(value >> 4));
    uart_send_hex_nibble(value);
}

static unsigned char fram_get_slave_address(unsigned long memory_address)
{
    unsigned char page_bit;

    page_bit = (unsigned char)((memory_address >> 16) & 0x01);

    return (unsigned char)(FRAM_I2C_BASE_ADDRESS | page_bit);
}

void fram_write_byte(unsigned long memory_address, unsigned char data)
{
    unsigned char slave_address;
    unsigned char address_high;
    unsigned char address_low;

    slave_address = fram_get_slave_address(memory_address);
    address_high = (unsigned char)((memory_address >> 8) & 0xFF);
    address_low = (unsigned char)(memory_address & 0xFF);

    while (UCB0STAT & UCBBUSY)
    {
    }

    UCB0I2CSA = slave_address;

    UCB0CTL1 |= UCTR | UCTXSTT;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0TXBUF = address_high;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0TXBUF = address_low;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0TXBUF = data;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0CTL1 |= UCTXSTP;

    while (UCB0CTL1 & UCTXSTP)
    {
    }
}

unsigned char fram_read_byte(unsigned long memory_address)
{
    unsigned char slave_address;
    unsigned char address_high;
    unsigned char address_low;
    unsigned char data;

    slave_address = fram_get_slave_address(memory_address);
    address_high = (unsigned char)((memory_address >> 8) & 0xFF);
    address_low = (unsigned char)(memory_address & 0xFF);

    while (UCB0STAT & UCBBUSY)
    {
    }

    UCB0I2CSA = slave_address;

    UCB0CTL1 |= UCTR | UCTXSTT;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0TXBUF = address_high;

    while (!(IFG2 & UCB0TXIFG))
    {
    }
    UCB0TXBUF = address_low;

    while (!(IFG2 & UCB0TXIFG))
    {
    }

    UCB0CTL1 &= ~UCTR;

    UCB0CTL1 |= UCTXSTT;

    while (UCB0CTL1 & UCTXSTT)
    {
    }

    UCB0CTL1 |= UCTXSTP;

    while (!(IFG2 & UCB0RXIFG))
    {
    }

    data = UCB0RXBUF;

    while (UCB0CTL1 & UCTXSTP)
    {
    }

    return data;
}