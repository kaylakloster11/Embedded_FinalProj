#include "msp.h"
#include "state.h"
#include <stdint.h>
#include "UART.h"
#include "port.h"
#include "timers.h"
#include "RTC.h"
#include "ADC.h"

/**
 * main.c
 */
//state var
volatile state_t state = IDLE;

//FPS state
volatile FP_cmd_packet cp;
volatile uint8_t FP_state;
volatile  uint8_t data_databuf[30];
volatile  uint8_t rsp_databuf[12];
volatile  FP_resp rp;
volatile uint8_t state3 = FP_DONT_CARE;
volatile uint8_t freqDiv = 0;
volatile uint8_t countTime = 0;
volatile uint8_t count_time_fall = 0;
volatile uint8_t timer_exit = 0;
volatile uint8_t FPS_byte_count = 0;

//accel globals
volatile uint16_t ACCELX=0;
volatile uint16_t ACCELY=0;
volatile uint16_t ACCELZ=0;

volatile uint16_t CALX=0;
volatile uint16_t CALY=0;
volatile uint16_t CALZ=0;

//button globals
#define SCB_SCR_ENABLE_SLEEPONEXIT (0x00000002)
volatile uint8_t pet = 0;
volatile uint8_t apps = 0;
volatile uint8_t lock = 0;
volatile uint8_t emergency = 0;


//flow globals
volatile uint8_t finger = 0;
volatile uint8_t print = 0;
volatile uint8_t fall = 0;

//timer globals
volatile uint32_t timer0 = 0;
volatile uint32_t timer1 = 0;

//RTC globals
volatile RealTime_t currentTime;
volatile uint8_t read = 0;
volatile uint8_t temp_sec = 61; //init out of range. This var is for debouncing.

volatile uint8_t RTC_str[4];

volatile uint8_t ADCcalib_end = 0;

void init_all() {
	config_buttons();
	config_buzzer();
	config_TA0();
	config_TA1();
	config_TA2();
	config_TA3();
	config_RTC();
	UART_ConfigureA2();
	UART_ConfigureA3();
	GPIO_FPS_config();
	configure_ADC();
	Open();
	SetLED(1);
}



void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    int begin = 0;
    int is_freefall = 0;
	init_all(); //inits all ports and timers and only enables fingerprint interrupts. i.e. only fingerprint stuff is running.

	while(state == IDLE) {
	    if(state3 == FP_DELETE1){
              FPS_Delete_ID();
              P2->OUT &= ~(BIT0 | BIT2);
              state3 = FP_DONT_CARE;
          }
          if(state3 == FP_DELETEALL){
              FPS_Delete_ALL();
              P2->OUT &= ~(BIT0 | BIT2) ;
              state3 = FP_DONT_CARE;
          }
          if(state3 == FP_ENROLL){
              ENROLLPRINT();
              P2->OUT &= ~(BIT2);
              state3 = FP_DONT_CARE;
          }

     // Identify fingerprint test

          while(IsFingerPressed() == 0){
              freqDiv = 30;
              TIMER_A2->CCR[0] = 50000;
              TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt enabled @ peripheral  (CCIE flag enabled)

              if(state3 == FP_DELETE1){
                  TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
                  FPS_Delete_ID();
                  P2->OUT &= ~(BIT0 | BIT2);
                  state3 = FP_DONT_CARE;
                  TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;
              }
              if(state3 == FP_DELETEALL){
                  TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
                  FPS_Delete_ALL();
                  P2->OUT &= ~(BIT0 | BIT2) ;
                  state3 = FP_DONT_CARE;
                  TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;
              }
              if(state3 == FP_ENROLL){
                  TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
                  ENROLLPRINT();
                  P2->OUT &= ~(BIT2);
                  state3 = FP_DONT_CARE;
                  TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;
              }
          }

              TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;

              CaptureFinger(1);
              int id = ID_print();
              char strID[3] = 0;
              itoa(id, strID);
              uint32_t lenID = strlen((char *)strID);

              if (id < 20)
              {//if the fingerprint matches, provide the matching template ID
                  UART_send_A3("Verified ID: ", 13);
                  UART_send_A3(&strID,lenID);
                  UART_send_byteA3(0x0A);
                  SetLED(0);
                  state = ACTIVE;
                  P3->SEL0 &= ~(BIT2 | BIT3); //reset UART2 pins to GPIO
                  P3->SEL1 &= ~(BIT2 | BIT3);
                  NVIC_DisableIRQ(EUSCIA2_IRQn); //disable FPS UART2
              }
              else
              {//if unable to recognize
                  UART_send_A3("Finger not found\n", 17);
              }
        }


	    UART_send_A3("Please Wait for Calibration\n", 28);
	    freqDiv = 30;
	    TIMER_A2->CCR[0] = 50000;
	    TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;   // Capture/Compare interrupt enabled @ peripheral  (CCIE flag enabled)
       while(!ADCcalib_end);
       ADCcalib_end  = 0;
       UART_send_A3("Calibration Finished\n", 21);
       SCB->SCR &= ~SCB_SCR_ENABLE_SLEEPONEXIT;
       ADC14->CTL0 |= ADC14_CTL0_ENC; //Enable Conversion
       ADC14->CTL0 |= ADC14_CTL0_SC;
       __sleep();
       calibration();
       ADC14->CTL0 &= ~ADC14_CTL0_SC;

       P4->IE |= (BIT4); //activate task buttons
       P5->IE |= (BIT7);
       P6->IE |= (BIT6);

	while(1) {
	    ADC14->CTL0 |= ADC14_CTL0_SC;
          __sleep();
        if (ACCELY < (CALY*0.7)){
            TIMER_A3->CCTL[0] |= TIMER_A_CCTLN_CCIE;
            timer_exit = 1;
            while(timer_exit);
             count_time_fall = 0;
             ADC14->CTL0 |= ADC14_CTL0_SC;
              __sleep();
              if (ACCELY < (CALY*0.7)){
                fall = 1;
               }
               else{
                 fall = 0;
               }
           }

	    handle_state();
		//accel code

		if(print) { //means timer triggered
			print = 0;
			sendCurrentTime();
			PrintReminders(GetButtonState(pet,apps,lock));
			UART_send_byteA3('\n');
		}
		if(fall) {
		    state = SOS;
		    UART_send_A3("In Freefall\n", 12);
		    UART_send_A3("Press Red Button If False Alarm\n", 32);
		    temp_sec = currentTime.sec;
		    P2->IFG = 0;
            P2->IE |= (BIT7); //turn on SOS button interrupt
		    TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE; //enable timer
		    TIMER_A1->CCR[0] = BUZZ_FREQ_SOS;
		    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE; //enable buzzer
		}

		if(emergency) {
                UART_send_A3("Emergency Detected\n", 19);
                UART_send_A3("Calling 911 ...\n", 16);
                P2->IE &= ~BIT7;
                pet = 0;
                apps = 0;
                lock = 0;
                emergency = 0;
		}
		if(read) {
		    read = 0;
		    getCurrentTime();
		}
		fall = 0;
	}



}
