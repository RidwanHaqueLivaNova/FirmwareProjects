// PROJECT: UART COMMAND APP WITH BUZZER LOGGING

#include "io430.h"
#include "event_logger.h"

#define RX_BUFFER_SIZE 32

#define BEEP_REPEAT_PERIOD_MS   3000
#define BEEP_BURST_DURATION_MS  5

static volatile unsigned char rx_buffer[RX_BUFFER_SIZE];
static volatile unsigned char rx_head = 0;
static volatile unsigned char rx_tail = 0;

static volatile unsigned char one_ms_tick = 0;
static volatile unsigned char beep_enabled = 0;


static volatile unsigned int beep_timer_ms = 0;


void hardware_init(void);
void uart_send_string(const char s[]);

static void buzzer_init(void)
{
    // BZR is connected to P1.5.
    // Configure P1.5 as GPIO output.
    P1DIR |= BIT5;

    // Start with buzzer off.
    P1OUT &= ~BIT5;
}

static void timer_init(void)
{
    // Configure Timer_A to interrupt every 1 ms.
    // Assumption: SMCLK is approximately 1 MHz.
    TACCR0 = 1000 - 1;

    // Enable Timer_A CCR0 interrupt.
    TACCTL0 = CCIE;

    // Timer_A source = SMCLK, up mode, clear timer.
    TACTL = TASSEL_2 | MC_1 | TACLR;
}

static void rx_buffer_push(unsigned char c)
{
    unsigned char next_head;

    next_head = (rx_head + 1) % RX_BUFFER_SIZE;

    // If buffer is full, drop the new character.
    if (next_head != rx_tail)
    {
        rx_buffer[rx_head] = c;
        rx_head = next_head;
    }
}

static unsigned char rx_buffer_available(void)
{
    unsigned char local_head;
    unsigned char local_tail;

    local_head = rx_head;
    local_tail = rx_tail;

    return local_head != local_tail;
}

static unsigned char rx_buffer_pop(void)
{
    unsigned char c;

    c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;

    return c;
}

static void process_command(unsigned char command)
{
    if ((command == 'b') || (command == 'B'))
    {
        beep_enabled = 1;
        beep_timer_ms = 0;

        uart_send_string("BEEP ON QUIET\r\n");

        logging_log_event(LOG_EVENT_BEEP_ON);
    }
    else if ((command == 's') || (command == 'S'))
    {
        beep_enabled = 0;
        beep_timer_ms = 0;
        P1OUT &= ~BIT5;

        uart_send_string("BEEP OFF\r\n");

        logging_log_event(LOG_EVENT_BEEP_OFF);
    }
    else if ((command == '\r') || (command == '\n'))
    {
        // Ignore Enter/newline characters.
    }
    else
    {
        uart_send_string("UNKNOWN COMMAND\r\n");
    }
}

#pragma vector=USCIAB1RX_VECTOR
__interrupt void USCI_A1_RX_ISR(void)
{
    unsigned char received;

    if ((UC1IFG & UCA1RXIFG) != 0)
    {
        // Reading RXBUF gets the received byte and clears the RX flag.
        received = UCA1RXBUF;

        // Keep ISR short. Store data and process it in main loop.
        rx_buffer_push(received);
    }
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMER_A0_ISR(void)
{
    one_ms_tick = 1;

    if (beep_enabled)
    {
        beep_timer_ms++;

        if (beep_timer_ms >= BEEP_REPEAT_PERIOD_MS)
        {
            beep_timer_ms = 0;
        }

        if (beep_timer_ms < BEEP_BURST_DURATION_MS)
        {
            // Very short chirp only.
            P1OUT ^= BIT5;
        }
        else
        {
            // Keep buzzer off most of the time.
            P1OUT &= ~BIT5;
        }
    }
    else
    {
        beep_timer_ms = 0;
        P1OUT &= ~BIT5;
    }

    // Wake CPU from low-power mode after ISR exits.
    __bic_SR_register_on_exit(LPM0_bits);
}

void command_app_run(void)
{
    hardware_init();
    buzzer_init();
    timer_init();

    // Enable UCA1 receive interrupt.
    UC1IE |= UCA1RXIE;

    uart_send_string("RDY\r\n");
    uart_send_string("Send b to start beep, s to stop beep\r\n");

    // Enable global interrupts.
    __enable_interrupt();

    while (1)
    {
        // Sleep until Timer_A wakes CPU every 1 ms.
        __bis_SR_register(LPM0_bits | GIE);

        if (one_ms_tick)
        {
            one_ms_tick = 0;

            while (rx_buffer_available())
            {
                unsigned char command;

                command = rx_buffer_pop();
                process_command(command);
            }
        }
    }
}