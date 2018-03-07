/*
 * port.c
 *
 *  Created on: Nov 28, 2017
 *      Author: admin
 */
#include "msp.h"
#include "port.h"
#include "UART.h"
#include "RTC.h"
#include "state.h"
#include "timers.h"


#define G_len (11)
#define B_len (12)
#define W_len (16)

extern volatile uint8_t print;
const uint8_t G_phrase[] = "Feed Pets!\n";
const uint8_t B_phrase[] = "Lock Doors!\n";
const uint8_t W_phrase[] = "Turn Off Stove!\n";

extern volatile RealTime_t currentTime;
extern volatile state_t state;

extern volatile uint8_t state3;

extern volatile uint8_t temp_sec;

extern volatile uint8_t pet;
extern volatile uint8_t apps;
extern volatile uint8_t lock;
extern volatile uint8_t emergency;

extern volatile uint8_t finger;

extern volatile uint8_t fall;

void config_buttons() {
	//SOS BUTTON -- Red
	P2->SEL0 &= ~BIT7;
	P2->SEL1 &= ~BIT7;
	P2->DIR &= ~(BIT7); //input
	P2->REN |= (BIT7); //Enabled pullup/pulldown resistors
	P2->OUT |= (BIT7); //Pullup selected
	P2->IFG = 0; //Clears interrupt flags
	P2->IES |= (BIT7); //Set high to low transition

	NVIC_EnableIRQ(PORT2_IRQn);

	//APPS BUTTON -- Green
	P4->SEL0 &= ~BIT4;
	P4->SEL1 &= ~BIT4;
	P4->DIR &= ~(BIT4); //input
	P4->REN |= (BIT4); //Enabled pullup/pulldown resistors
	P4->OUT |= (BIT4); //Pullup selected
	P4->IFG = 0; //Clears interrupt flags
	P4->IES |= (BIT4); //Set high to low transition

	NVIC_EnableIRQ(PORT4_IRQn);

	//LOCKS BUTTON -- blue
	P5->SEL0 &= ~BIT7;
	P5->SEL1 &= ~BIT7;
	P5->DIR &= ~(BIT7); //input
	P5->REN |= (BIT7); //Enabled pullup/pulldown resistors
	P5->OUT |= (BIT7); //Pullup selected
	P5->IFG = 0; //Clears interrupt flags
	P5->IES |= (BIT7); //Set high to low transition

	NVIC_EnableIRQ(PORT5_IRQn);

	//PETS BUTTON -- black
	P6->SEL0 &= ~BIT6;
	P6->SEL1 &= ~BIT6;
	P6->DIR &= ~(BIT6); //input
	P6->REN |= (BIT6); //Enabled pullup/pulldown resistors
	P6->OUT |= (BIT6); //Pullup selected
	P6->IFG = 0; //Clears interrupt flags
	P6->IES |= (BIT6); //Set high to low transition

	NVIC_EnableIRQ(PORT6_IRQn);
}

void config_buzzer() {
	//config buzzer
	P3->SEL0 &= ~BIT0;
	P3->SEL1 &= ~BIT0;
	P3->DIR |= BIT0; //output
	P3->OUT &= ~BIT0; //low
}

REMINDER_STATUS GetButtonState(uint8_t G_ID, uint8_t B_ID, uint8_t W_ID){
	REMINDER_STATUS state;
	switch(G_ID)
	{
		case 1:
		if(B_ID){
			if(W_ID){
				state = GWB;
			}else 	{
			state = GB;
			}
		}
		else{
			if(W_ID){
				state = GW;
			}
			else{
				state = G;
			}
		}
		break;
		case 0:
			if(B_ID){
				if(W_ID){
					state = BW;
				}
				else{
					state = B;
				}
			}
			else{
				if(W_ID){
					state = W;
				}
				else{
					state = NONE;
				}
			}
			break;
	}
		return state;
}

void PrintReminders(REMINDER_STATUS status){
	switch(status)
	{
		case GWB:
			UART_send_A3(G_phrase, G_len);
			UART_send_A3(B_phrase, B_len);
			UART_send_A3(W_phrase, W_len);
			UART_send_byteA3(0x0A);
			break;

		case G:
			UART_send_A3(G_phrase, G_len);
			UART_send_byteA3(0x0A);
			break;

		case B:
			UART_send_A3(B_phrase, B_len);
			UART_send_byteA3(0x0A);
			break;

		case W:
			UART_send_A3(W_phrase, W_len);
			UART_send_byteA3(0x0A);
			break;

		case GB:
			UART_send_A3(G_phrase, G_len);
			UART_send_A3(B_phrase, B_len);
			UART_send_byteA3(0x0A);
			break;

		case GW:
			UART_send_A3(G_phrase, G_len);
			UART_send_A3(W_phrase, W_len);
			UART_send_byteA3(0x0A);
			break;

		case BW:
			UART_send_A3(B_phrase, B_len);
			UART_send_A3(W_phrase, W_len);
			UART_send_byteA3(0x0A);
			break;

		default:
				break;
	}
	return;
}

void PORT2_IRQHandler(void){
    if(currentTime.sec == temp_sec) {
        P2->IFG = 0;
        return;
    }
	if((state == SOS) && (temp_sec != currentTime.sec)) {
	    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
		temp_sec = currentTime.sec;
		TIMER_A1->CCR[0] = BUZZ_FREQ_TASK;
		state = ACTIVE;
	}
	else if((temp_sec == currentTime.sec) || temp_sec+1 == currentTime.sec){
		P2->IFG = 0;
		return;
	}
	else {
		TIMER_A1->CCR[0] = BUZZ_FREQ_TASK; //set buzz freq back to normal
		state = ACTIVE;
	}
	P2->IFG = 0;
}

void PORT4_IRQHandler(void){
	if((apps) && (temp_sec != currentTime.sec)) {
		apps = 0;
		temp_sec = currentTime.sec;
	}
	else if((temp_sec == currentTime.sec) || temp_sec+1 == currentTime.sec){
		P4->IFG = 0;
		return;
	}
	else {
		apps = 1;
		temp_sec = currentTime.sec;
	}
	P4->IFG = 0;
}

void PORT5_IRQHandler(void){
	if((lock) && (temp_sec != currentTime.sec)) {
		lock = 0;
		temp_sec = currentTime.sec;
	}
	else if((temp_sec == currentTime.sec) || temp_sec+1 == currentTime.sec){
		P5->IFG = 0;
		return;
	}
	else {
		lock = 1;
		temp_sec = currentTime.sec;
	}
	P5->IFG = 0;
}
void PORT6_IRQHandler(void){
	if((pet) && (temp_sec != currentTime.sec)) {
		pet = 0;
		temp_sec = currentTime.sec;
	}
	else if((temp_sec == currentTime.sec) || temp_sec+1 == currentTime.sec){
		P6->IFG = 0;
		return;
	}
	else {
		pet = 1;
		temp_sec = currentTime.sec;
	}
	P6->IFG = 0;
}
/****************************************************************************************************************************/


void GPIO_FPS_config(){
   /* Left and right button configure */
      P1->SEL0 &= ~(BIT1 | BIT4);     //Use the OR function to set BIT1 and BIT4 to general IO Mode
      P1->SEL1 &= ~(BIT1 | BIT4) ;     //^^^
      P1->DIR &= ~(BIT1 | BIT4);       //Direction pin is set to zero since BIT1 and BIT4 are inputs
      P1->REN |= BIT1 | BIT4 ;        //REN is set to 1 at BIT1 and BIT4 in order to enable pullup/pulldown
      P1->OUT |= BIT1 | BIT4 ;        //Output resgister is set to 1 to enable the pullup resister
      P1->IE |= BIT1 | BIT4 ;          //interrupts are enabled
      P1->IES |= BIT1  | BIT4;        //high-to-low transition
      P1->IFG = 0;                    //interrupt flag is set to zero
      P2->SEL0 &= ~(BIT0 | BIT2| BIT1) ;     //GENERAL I/O for RED and BLUE
      P2->SEL1 &= ~(BIT0 | BIT2 | BIT1) ;     //^^^
      P2->OUT &= ~(BIT0 | BIT2 | BIT1) ;      //Direction is set to 1 so the LEDs act as outputs
      P2->DIR |= (BIT0 | BIT2 | BIT1);         //Output is set to low so the LED will only turn on when the interrupt is called


       /* Enable Interrupts in the NVIC */
      NVIC_EnableIRQ(PORT1_IRQn);
 }
void PORT1_IRQHandler(){

	int i = 0;
	for(i=0; i<= 10; i++); //delay to allow for debouncing

	if(state3 == FP_DELETE){
		if(P1->IFG & BIT1) {
			 state3 = FP_DELETE1;
			 P1->IFG &= ~BIT1; //clear flag
		 }
		if(P1->IFG & BIT4) {
			state3 = FP_DELETEALL;
			P1->IFG &= ~BIT4; //clear flag
		}
	 }
	else{
		if(P1->IFG & BIT1) {
			state3 = FP_DELETE;
			P2->OUT |= (BIT0 | BIT2);
			P1->IFG &= ~BIT1; //clear flag
		}
		if(P1->IFG & BIT4) {
			state3 = FP_ENROLL;
			P2->OUT |= (BIT2);
			P1->IFG &= ~BIT4; //clear flag
		}
	}
	}



