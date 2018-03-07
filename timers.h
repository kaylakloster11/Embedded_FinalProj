/*
 * timers.h
 *
 *  Created on: Oct 26, 2017
 *      Author: derek
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#include <stdint.h>
#include "msp.h"

#define TASK_TIMER_CYCLES      (10)
#define SOS_TIMER_CYCLES       (30)
#define BUZZER_CYCLES        (1000)
#define BUZZ_FREQ_TASK       (2000)
#define BUZZ_FREQ_SOS        (3000)

void config_TA0(void);
void config_TA1(void);
void config_TA2(void);
void config_TA3(void);

void TA0_0_IRQHandler();
void TA1_0_IRQHandler();
void TA2_0_IRQHandler();
void TA3_0_IRQHandler();

#endif /* TIMERS_H_ */
