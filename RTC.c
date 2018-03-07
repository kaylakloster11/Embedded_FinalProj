/*
 * RTC.c
 *
 *  Created on: Nov 28, 2017
 *      Author: admin
 */
#include "msp.h"
#include "RTC.h"
#include "UART.h"


extern volatile uint8_t read;
extern volatile uint8_t temp_sec; //init out of range. This var is for debouncing.
extern volatile uint8_t RTC_str[4];

void config_RTC() {

	RTC_C->CTL0 = RTC_C_KEY | RTC_C_CTL0_RDYIE; // unlock RTC
	RTC_C->CTL13 |= (RTC_C_CTL13_HOLD | RTCBCD);

	RTC_C->YEAR = YEAR_INIT;                   // Year = 0x2017
	RTC_C->DATE = (MONTH_INIT << RTC_C_DATE_MON_OFS) | // Month = 0x12 = Dec
			(DOM_INIT | RTC_C_DATE_DAY_OFS);    // Day = 0x05 = 5th
	RTC_C->TIM1 = (DOW_INIT << RTC_C_TIM1_DOW_OFS) | // Day of week = 0x06 = Sat
			(HR_INIT << RTC_C_TIM1_HOUR_OFS);  // Hour = 0x10
	RTC_C->TIM0 = (MIN_INIT << RTC_C_TIM0_MIN_OFS) | // Minute = 0x32
			(SEC_INIT << RTC_C_TIM0_SEC_OFS);   // Seconds = 0x45

	// Start RTC calendar mode
	RTC_C->CTL13 &= ~(RTC_C_CTL13_HOLD);

	// Lock the RTC registers
	RTC_C->CTL0 &= ~(RTC_C_CTL0_KEY_MASK);

	NVIC_EnableIRQ(RTC_C_IRQn);
}

void RTC_C_IRQHandler() {
	if(RTC_C->CTL0 & RTCRDYIFG) {
		RTC_C->CTL0 = RTC_C_KEY;
		RTC_C->CTL0 = (RTC_C_KEY | RTCRDYIE); //clear flag
		read = 1;
	}
}

void getCurrentTime(){
	currentTime.year = (RTC_C->YEAR >> 12)*1000 + ((RTC_C->YEAR>>8)%16)*100 + ((RTC_C->YEAR>>4)%16)*10 + RTC_C->YEAR%16;
	currentTime.month = (RTC_C->DATE >> 12)*10 + ((RTC_C->DATE >> 8) % 16);
	currentTime.dom = ((RTC_C->DATE % (1 << RTC_C_DATE_MON_OFS))>>4)*10 + (RTC_C->DATE % 16);
	currentTime.hour = ((RTC_C->TIM1 % (1 << DAYOFWEEK_OFS)) >> 4)*10 + (RTC_C->TIM1 % (1 << DAYOFWEEK_OFS)) % 16;
	currentTime.min = ((RTC_C->TIM0 >> RTC_C_TIM0_MIN_OFS)>>4)*10 + (RTC_C->TIM0 >> RTC_C_TIM0_MIN_OFS) % 16;
	currentTime.sec = ((RTC_C->TIM0 % (1 << RTC_C_TIM0_MIN_OFS))>>4)*10 + (RTC_C->TIM0 % (1 << RTC_C_TIM0_MIN_OFS))%16;
}

void sendCurrentTime() {
	getCurrentTime();
	itoa(currentTime.year, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 4);
	UART_send_byteA3('-');
	itoa(currentTime.month, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 2);
	UART_send_byteA3('-');
	itoa(currentTime.dom, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 2);
	UART_send_byteA3(',');
	UART_send_byteA3(' ');
	itoa(currentTime.hour, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 2);
	UART_send_byteA3(':');
	itoa(currentTime.min, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 2);
	UART_send_byteA3(':');
	itoa(currentTime.sec, (char*)RTC_str);
	UART_send_A3((const uint8_t*)RTC_str, 2);
	UART_send_byteA3('-');
	UART_send_byteA3('-');
	UART_send_byteA3('\r');
}
