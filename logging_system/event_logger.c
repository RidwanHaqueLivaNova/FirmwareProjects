// SIMPLE FRAM EVENT LOGGER TEST

#include "io430.h"
#include "event_logger.h"

#define LOG_START_ADDRESS   0x000200UL
#define LOG_ENTRY_SIZE      8

#define LOG_MARKER          0xA5
#define LOG_EVENT_TEST_1    0x11
#define LOG_EVENT_TEST_2    0x22




void uart_send_string(const char s[]);
void uart_send_hex_byte(unsigned char value);
void fram_write_byte(unsigned long memory_address, unsigned char data);
unsigned char fram_read_byte(unsigned long memory_address);

static unsigned int log_sequence = 0;
static unsigned long log_next_address = LOG_START_ADDRESS;

static unsigned char log_calculate_checksum(unsigned char marker,
                                            unsigned char event_id,
                                            unsigned char sequence_low,
                                            unsigned char sequence_high,
                                            unsigned char data0,
                                            unsigned char data1,
                                            unsigned char data2)
{
    unsigned char checksum;

    checksum = 0;
    checksum += marker;
    checksum += event_id;
    checksum += sequence_low;
    checksum += sequence_high;
    checksum += data0;
    checksum += data1;
    checksum += data2;

    return checksum;
}






static void logging_write_event(unsigned char event_id)
{
    unsigned char sequence_low;
    unsigned char sequence_high;
    unsigned char data0;
    unsigned char data1;
    unsigned char data2;
    unsigned char checksum;
    unsigned long address;

    sequence_low = (unsigned char)(log_sequence & 0xFF);
    sequence_high = (unsigned char)((log_sequence >> 8) & 0xFF);

    data0 = 0;
    data1 = 0;
    data2 = 0;

    checksum = log_calculate_checksum(LOG_MARKER,
                                      event_id,
                                      sequence_low,
                                      sequence_high,
                                      data0,
                                      data1,
                                      data2);

    address = log_next_address;

    fram_write_byte(address + 0, LOG_MARKER);
    fram_write_byte(address + 1, event_id);
    fram_write_byte(address + 2, sequence_low);
    fram_write_byte(address + 3, sequence_high);
    fram_write_byte(address + 4, data0);
    fram_write_byte(address + 5, data1);
    fram_write_byte(address + 6, data2);
    fram_write_byte(address + 7, checksum);

    log_sequence++;
    log_next_address += LOG_ENTRY_SIZE;
}


void logging_log_event(unsigned char event_id)
{
  logging_write_event(event_id);
  uart_send_string("EVENT LOGGED: ");
  uart_send_hex_byte(event_id);
  uart_send_string("\r\n");
  
}





static unsigned char logging_verify_event(unsigned long address,
                                          unsigned char expected_event_id)
{
    unsigned char marker;
    unsigned char event_id;
    unsigned char sequence_low;
    unsigned char sequence_high;
    unsigned char data0;
    unsigned char data1;
    unsigned char data2;
    unsigned char stored_checksum;
    unsigned char calculated_checksum;

    marker = fram_read_byte(address + 0);
    event_id = fram_read_byte(address + 1);
    sequence_low = fram_read_byte(address + 2);
    sequence_high = fram_read_byte(address + 3);
    data0 = fram_read_byte(address + 4);
    data1 = fram_read_byte(address + 5);
    data2 = fram_read_byte(address + 6);
    stored_checksum = fram_read_byte(address + 7);

    calculated_checksum = log_calculate_checksum(marker,
                                                 event_id,
                                                 sequence_low,
                                                 sequence_high,
                                                 data0,
                                                 data1,
                                                 data2);

    if (marker != LOG_MARKER)
    {
        return 0;
    }

    if (event_id != expected_event_id)
    {
        return 0;
    }

    if (stored_checksum != calculated_checksum)
    {
        return 0;
    }

    return 1;
}

static void logging_print_event(unsigned long address)
{
    unsigned char marker;
    unsigned char event_id;
    unsigned char sequence_low;
    unsigned char sequence_high;
    unsigned char checksum;

    marker = fram_read_byte(address + 0);
    event_id = fram_read_byte(address + 1);
    sequence_low = fram_read_byte(address + 2);
    sequence_high = fram_read_byte(address + 3);
    checksum = fram_read_byte(address + 7);

    uart_send_string("Log entry at address ");
    uart_send_hex_byte((unsigned char)((address >> 8) & 0xFF));
    uart_send_hex_byte((unsigned char)(address & 0xFF));
    uart_send_string(": marker=");
    uart_send_hex_byte(marker);
    uart_send_string(" event=");
    uart_send_hex_byte(event_id);
    uart_send_string(" seq=");
    uart_send_hex_byte(sequence_high);
    uart_send_hex_byte(sequence_low);
    uart_send_string(" checksum=");
    uart_send_hex_byte(checksum);
    uart_send_string("\r\n");
}

void event_logger_test_run(void)
{
    unsigned char verify_1;
    unsigned char verify_2;

    uart_send_string("EVENT LOGGER TEST START\r\n");

    log_sequence = 0;
    log_next_address = LOG_START_ADDRESS;

    logging_write_event(LOG_EVENT_TEST_1);
    logging_write_event(LOG_EVENT_TEST_2);

    logging_print_event(LOG_START_ADDRESS);
    logging_print_event(LOG_START_ADDRESS + LOG_ENTRY_SIZE);

    verify_1 = logging_verify_event(LOG_START_ADDRESS, LOG_EVENT_TEST_1);
    verify_2 = logging_verify_event(LOG_START_ADDRESS + LOG_ENTRY_SIZE, LOG_EVENT_TEST_2);

    if (verify_1 && verify_2)
    {
        uart_send_string("EVENT LOGGER TEST PASS\r\n");
    }
    else
    {
        uart_send_string("EVENT LOGGER TEST FAIL\r\n");
    }

    uart_send_string("EVENT LOGGER TEST DONE\r\n");
}