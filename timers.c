/*
 * timers.c
 *
 *  Created on: Oct 26, 2017
 *      Author: derek
 */
#include "port.h"
#include "timers.h"
#include "UART.h"
#include "state.h"

extern volatile uint8_t freqDiv;
extern volatile uint8_t countTime;

extern volatile uint8_t count_time_fall;
extern volatile uint8_t timer_exit;

extern volatile state_t state;
extern volatile uint8_t emergency;

extern volatile uint8_t print;

extern volatile uint32_t timer0;
extern volatile uint32_t timer1;

extern volatile uint8_t ADCcalib_end;
//Timer for reminders. Should be set for div64, meaning every interrupt is every 1.4 seconds.
//Have a global var for making it like 10 second reminders for demo.
//Interrupt enabled in update_state() based on buttons
void config_TA0() {
	//timer stuff
	TIMER_A0->R = 0; // Reset count

	// SMCLK, Up mode, interrupts enabled, div set to 8,
	TIMER_A0->CCR[0] = 65535;    // Value to count to; for 1.4 second interrupts
	TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID__8;
	TIMER_A0->EX0 |= TIMER_A_EX0_TAIDEX_7; //divide again by 8

	/* Enable Interrupts in the NVIC */
	NVIC_EnableIRQ(TA0_0_IRQn);
}

//This will be buzzer timer
//Interrupt enabled in the button timer, handler, as well as in the event of a fall.
void config_TA1() {
	TIMER_A1->R = 0; // Reset count

	// SMCLK, Up mode, interrupts enabled, div set to 8,
	TIMER_A1->CCR[0] = BUZZ_FREQ_TASK;// optimal beeping frequency
	TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP;

	/* Enable Interrupts in the NVIC */
	NVIC_EnableIRQ(TA1_0_IRQn);
}
//To be used with FPS at startup
void config_TA2() {
    TIMER_A2->R = 0 ;         // Reset count
    TIMER_A2->CTL |= (TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__8 | TIMER_A_CTL_MC_1 | TIMER_A_CTL_IE ) ; //Set SMCLK Multiplexer mode clock source, divide by 1
        // and Up mode (BIT5,BIT4 = 01) and enable interrupt for timer register (enable TAIE flag)
    TIMER_A2->EX0 |= TIMER_A_EX0_TAIDEX_7; //divide again by 8
    TIMER_A2->CCR[0] = 37500;    // Value to count to TAxCCRn holds the data for the comparison to the timer value
          //in the Timer_A Register, TAxR.
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt enabled @ peripheral  (CCIE flag enabled)
    TIMER_A2->CTL &= ~(TIMER_A_CCTLN_CAP); // set BIT8 to zero to set to compare mode (CAP = 0)
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG ; //Clear CCIFG interrupt (BIT0)
    TIMER_A2->CTL &= ~TIMER_A_CTL_IFG;  //Clear TAIFG interrupt (BIT0)
    /* Enable Interrupts in the NVIC */
      NVIC_EnableIRQ(TA2_0_IRQn );
 }


//For accel, fall detection
void config_TA3(){
    TIMER_A3->R = 0 ;         // Reset count
    TIMER_A3->CCR[0] = 37500;    // Value to count to TAxCCRn holds the data for the comparison to the timer value
             //in the Timer_A Register, TAxR.
    TIMER_A3->CTL |= (TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__8 | TIMER_A_CTL_MC_1) ; //Set SMCLK Multiplexer mode clock source, divide by 1
        // and Up mode (BIT5,BIT4 = 01) and enable interrupt for timer register (enable TAIE flag)

    TIMER_A3->EX0 |= TIMER_A_EX0_TAIDEX_7; //divide again by 8
    TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt enabled @ peripheral  (CCIE flag enabled)
    TIMER_A3->CTL &= ~(TIMER_A_CCTLN_CAP); // set BIT8 to zero to set to compare mode (CAP = 0)
    TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG ; //Clear CCIFG interrupt (BIT0)
    /* Enable Interrupts in the NVIC */
      NVIC_EnableIRQ(TA3_0_IRQn);
  }

void TA0_0_IRQHandler() {
	TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear flag
	timer0++;
	if(state == SOS) {
	    if(timer0 == SOS_TIMER_CYCLES) {
	        TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
	        emergency = 1;
	        timer0 = 0;
	    }
	}
	else {
        if(timer0 == TASK_TIMER_CYCLES) { //CYCLES*1.4 seconds before print.
            print = 1;
            timer0 = 0;
            TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE; // enable buzzer
        }
	}
}

void TA1_0_IRQHandler() {
	TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear flag
	timer1++;
	if (timer1 >= BUZZER_CYCLES) {
		timer1 = 0;
		TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE; //turn off buzzer
	}
	//buzzer pin
	P3->OUT ^= BIT0;
}

void TA2_0_IRQHandler() {
	TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
	countTime++;
	if(countTime == freqDiv) {
	   countTime = 0;
	    if(state == IDLE)
	   {
	       UART_send_A3("Please press finger\n", 20);
	   }
	   else{
	       ADCcalib_end = 1;
	   }

	   TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt disabled @ peripheral  (CCIE flag enabled)
	}
}

void TA3_0_IRQHandler()
{
    TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    count_time_fall++;
    if(count_time_fall == 20) {
        timer_exit = 0;
       count_time_fall = 0;
       TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt disabled @ peripheral  (CCIE flag enabled)
   }
}









