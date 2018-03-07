/*
 * ADC.h
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */

#ifndef ADC_H_
#define ADC_H_

#include "msp.h"
#include <stdint.h>

void configure_ADC();
void calibration();
int freefall();

extern volatile uint8_t count_time_fall;
extern volatile uint8_t timer_exit;

extern volatile uint16_t ACCELX;
extern volatile uint16_t ACCELY;
extern volatile uint16_t ACCELZ;

extern volatile uint16_t CALX;
extern volatile uint16_t CALY;
extern volatile uint16_t CALZ;

#endif /* ADC_H_ */
