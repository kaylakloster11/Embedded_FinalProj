/*
 * ADC.c
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */
#include "ADC.h"

extern volatile uint8_t count_time_fall;

extern volatile uint8_t timer_exit;

void configure_ADC(){
    //ACCELERATION PINS:
    //X direction accelerometer
     P6->SEL0 |= BIT1;
     P6->SEL1 |= BIT1;

     //Y direction accelerometer
     P4->SEL0 |= BIT0;
     P4->SEL1 |= BIT0;

     //Z direction accelerometer
     P4->SEL0 |= BIT2;
     P4->SEL1 |= BIT2;

    while(REF_A->CTL0 & REF_A_CTL0_GENBUSY);

    REF_A->CTL0 = REF_A_CTL0_VSEL_0 | REF_A_CTL0_ON;

    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    ADC14->CTL0 |= ADC14_CTL0_SHT0_5 | ADC14_CTL0_ON | ADC14_CTL0_SHP;
    ADC14->CTL0 |= ADC14_CTL0_CONSEQ_1;
    ADC14->CTL1 = ADC14_CTL1_RES_3;
  //  ADC14->CTL1 |= ADC14_CTL1_TCMAP;

    ADC14->MCTL[0] = ADC14_MCTLN_INCH_14 | ADC14_MCTLN_VRSEL_0;
    ADC14->MCTL[1] = ADC14_MCTLN_INCH_11 | ADC14_MCTLN_VRSEL_0;
    ADC14->MCTL[2] = ADC14_MCTLN_INCH_13 | ADC14_MCTLN_VRSEL_0 | ADC14_MCTLN_EOS;

    ADC14->IER0 |= ADC14_IER0_IE0 | ADC14_IER0_IE1 | ADC14_IER0_IE2;
    while(!(REF_A->CTL0 & REF_A_CTL0_GENRDY));
    NVIC_EnableIRQ(ADC14_IRQn);
}

void ADC14_IRQHandler(){

   if(ADC14_IFGR0_IFG0){
       ACCELX = ADC14->MEM[0];
      }
   if(ADC14_IFGR0_IFG1){
       ACCELY = ADC14->MEM[1];
   }
   if(ADC14_IFGR0_IFG2){
       ACCELZ = ADC14->MEM[2];
   }

}

void calibration(){
    //after fingerprint sensor turned on
    //prompt user to stand up straight
    CALX = ACCELX;
    CALY = ACCELY;
    CALZ = ACCELZ;

}

int freefall(){
    //fall forward
    int retval = 0;
    if (ACCELY < (CALY*0.7)){
     retval = 1;
     TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE;
    }
    else{
        retval = 0;
    }
    return retval;
}


