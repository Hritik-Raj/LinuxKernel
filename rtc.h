#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "i8259.h"
#include "keyboard.h"

#define RTC_IRQ 8		// RTC corresponds to IRQ line 
#define REG_A   0x8A
#define REG_B   0x8B
#define RTC_PORT 0x70
#define RTC_DATA 0x71
#define REG_C    0x0C
#define BASE_FREQ 32768
#define MIN_RATE 2
#define MAX_RATE 1024
#define NBYTES 4



void rtc_init();			// User defined functions
void rtc_handler();

extern int32_t rtc_open();
extern int32_t rtc_read();
extern int32_t rtc_write(const void* buf, int32_t nbytes);
extern int32_t rtc_close();


#endif /* RTC_H */
