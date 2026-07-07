// FRAM MEMORY WRITE/READ SELF TEST

#include "io430.h"

#define FRAM_TEST_ADDRESS 0x000100UL
#define FRAM_TEST_VALUE   0x5A

void uart_send_string(const char s[]);
void uart_send_hex_byte(unsigned char value);
void fram_write_byte(unsigned long memory_address, unsigned char data);
unsigned char fram_read_byte(unsigned long memory_address);

void fram_memory_test_run(void)
{
    unsigned char original_value;
    unsigned char readback_value;
    unsigned char restored_value;

    uart_send_string("FRAM MEMORY TEST START\r\n");

    original_value = fram_read_byte(FRAM_TEST_ADDRESS);

    uart_send_string("Original value: ");
    uart_send_hex_byte(original_value);
    uart_send_string("\r\n");

    fram_write_byte(FRAM_TEST_ADDRESS, FRAM_TEST_VALUE);

    readback_value = fram_read_byte(FRAM_TEST_ADDRESS);

    uart_send_string("Wrote value: ");
    uart_send_hex_byte(FRAM_TEST_VALUE);
    uart_send_string("\r\n");

    uart_send_string("Readback value: ");
    uart_send_hex_byte(readback_value);
    uart_send_string("\r\n");

    if (readback_value == FRAM_TEST_VALUE)
    {
        uart_send_string("FRAM WRITE/READ TEST PASS\r\n");
    }
    else
    {
        uart_send_string("FRAM WRITE/READ TEST FAIL\r\n");
    }

    fram_write_byte(FRAM_TEST_ADDRESS, original_value);

    restored_value = fram_read_byte(FRAM_TEST_ADDRESS);

    uart_send_string("Restored value: ");
    uart_send_hex_byte(restored_value);
    uart_send_string("\r\n");

    if (restored_value == original_value)
    {
        uart_send_string("FRAM RESTORE PASS\r\n");
    }
    else
    {
        uart_send_string("FRAM RESTORE FAIL\r\n");
    }

    uart_send_string("FRAM MEMORY TEST DONE\r\n");
}