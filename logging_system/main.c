// PROJECT: LOGGING SYSTEM

#include "io430.h"

void hardware_init(void);
void uart_send_string(const char s[]);
void fram_memory_test_run(void);
void event_logger_test_run(void);

int main(void)
{
    hardware_init();

    uart_send_string("RDY\r\n");

    fram_memory_test_run();

    event_logger_test_run();

    while (1)
    {
        // Tests complete.
        // Stay here.
    }
}