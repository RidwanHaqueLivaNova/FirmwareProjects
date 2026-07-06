#include "io430.h"

static void chip_init(void)
{
    // Stop watchdog timer for this simple standalone test.
    WDTCTL = WDTPW | WDTHOLD;

    // Clock setup based on your given/observed values.
    DCOCTL  = 0x60;
    BCSCTL1 = 0x87;
    BCSCTL2 = 0x00;
    BCSCTL3 = 0x05;
}

static void uart_init(void)
{
    // Put USCI_A1 into reset while configuring it.
    UCA1CTL1 |= UCSWRST;

    // Use SMCLK as UART clock source.
    UCA1CTL1 |= UCSSEL_2;

    // Baud rate settings for 9600 baud if SMCLK is approximately 1 MHz.
    UCA1BR0 = 104;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_1;

    // Confirmed working UART pins:
    // P3.6 = UCA1TXD, P3.7 = UCA1RXD.
    P3SEL |= BIT6 | BIT7;

    // Release USCI_A1 from reset.
    UCA1CTL1 &= ~UCSWRST;

    // Enable UCA1 receive interrupt.
    UC1IE |= UCA1RXIE;
}

static void uart_send_char(char c)
{
    while (!(UC1IFG & UCA1TXIFG))
    {
    }

    UCA1TXBUF = c;
}

#pragma vector=USCIAB1RX_VECTOR
__interrupt void USCI_A1_RX_ISR(void)
{
    char received;

    // Check that UCA1 RX interrupt flag is set.
    if ((UC1IFG & UCA1RXIFG) != 0)
    {
        // Reading RXBUF clears the RX flag.
        received = UCA1RXBUF;

        // Echo received character back.
        uart_send_char(received);
    }
}

int main(void)
{
    chip_init();
    uart_init();

    // Startup message.
    uart_send_char('R');
    uart_send_char('D');
    uart_send_char('Y');
    uart_send_char('\r');
    uart_send_char('\n');

    // Enable global interrupts.
    __enable_interrupt();

    while (1)
    {
        // Main loop intentionally empty.
        // UART receive interrupt handles echo.
    }
}