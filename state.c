/*
 * state.c
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */
#include "state.h"
#include "msp.h"

extern volatile state_t state;
extern volatile uint8_t pet;
extern volatile uint8_t apps;
extern volatile uint8_t lock;
extern volatile uint8_t emergency;


void update_state() {
	if (emergency){
        state = SOS;
	}
	else if(state == SOS)
	    state = SOS;
	else if(pet || apps || lock)
		state = TASK_ACTIVE;
	else
	    state = ACTIVE;
}

void handle_state() {
	update_state();
	if((state == TASK_ACTIVE) || (state == SOS))
		TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;
	else if(state == ACTIVE)
		TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
}
