/*
 * RTC.h
 *
 *  Created on: Nov 28, 2017
 *      Author: admin
 */
#ifndef RTC_H_
#define RTC_H_
#include <stdint.h>
#include "msp.h"

//RTC defines
#define YEAR_INIT 0x2017
#define MONTH_INIT 0x12
#define DOM_INIT 0x10
#define DOW_INIT 0x0
#define HR_INIT 0x16
#define MIN_INIT 0x19
#define SEC_INIT 0x00


//TIME TYPE
typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t dom;
	uint8_t dow;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}RealTime_t;

extern volatile RealTime_t currentTime;

void config_RTC();
void RTC_C_IRQHandler();
void sendCurrentTime();
void getCurrentTime();

#endif /* RTC_H_ */
