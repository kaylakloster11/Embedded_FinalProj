/*
 * state.h
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */

#ifndef STATE_H_
#define STATE_H_
#include <stdint.h>
typedef enum {
	IDLE,
	ACTIVE,
	TASK_ACTIVE,
	SOS
} state_t;

void update_state();
void handle_state();

#endif /* STATE_H_ */
