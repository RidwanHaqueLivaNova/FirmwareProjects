#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#define LOG_EVENT_BEEP_ON   0x01
#define LOG_EVENT_BEEP_OFF  0x02


void logging_init(void);
void logging_log_event(unsigned char event_id);
void logging_dump_events(void);
void logging_clear(void);

#endif