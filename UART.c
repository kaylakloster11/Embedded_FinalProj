/*
 * UART.c
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */

#include "UART.h"

extern volatile uint8_t FP_state;
uint8_t i;
extern volatile  uint8_t data_databuf[30];
extern volatile  uint8_t rsp_databuf[12];
extern volatile uint8_t FPS_byte_count;

void reverse(char s[])
 {
	 int i, j;
	 char c;
	 for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		 c = s[i];
		 s[i] = s[j];
		 s[j] = c;
	 }
 }

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

 void EUSCIA2_IRQHandler(){
     if(EUSCI_A2->IFG & EUSCI_A_IFG_RXIFG){
         if (FP_state == FP_WAIT_RSP)
             {
                 rsp_databuf[FPS_byte_count++]= EUSCI_A2->RXBUF;
                 if(FPS_byte_count == FP_RSP_PACKET_LENGTH)
                     {
                         FP_state = FP_PROCESS_RSP;
                         EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG;
                     }
             }
         else if (FP_state == FP_WAIT_DATA)
             {
                 data_databuf[FPS_byte_count++] = EUSCI_A2->RXBUF;
                 if(FPS_byte_count == FP_DATA_PACKET_LENGTH )
                     {
                         FP_state = FP_PROCESS_RSP;
                         EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG;
                     }
             }
         }
     }


void UART_send_byteA2(uint8_t data){
   while((EUSCI_A2->STATW & EUSCI_A_STATW_BUSY)); //May have to change to using the TXIFG flag
        EUSCI_A2->TXBUF = data;
}

void UART_send_A2(const uint8_t * data, uint32_t length){
    int i = 0;
    for(i=0 ; i<length ; i++){
        UART_send_byteA2(data[i]);
    }
}

void UART_send_A3(const uint8_t * data, uint32_t length){
    int i = 0;
    for(i=0 ; i<length ; i++){
        UART_send_byteA3(data[i]);
    }
}

void UART_send_byteA3(uint8_t data){
   while((EUSCI_A3->STATW & EUSCI_A_STATW_BUSY)); //May have to change to using the TXIFG flag
        EUSCI_A3->TXBUF = data;
}


void UART_ConfigureA3(){

    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
    CS_CTL1_SELS_3 |                // SMCLK = DCO
    CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    P9->SEL0 |= BIT6 | BIT7; //configure RX/TX Pins 1.2/1.3
    P9->SEL1 &= ~(BIT6 | BIT7);

       EUSCI_A3->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
       EUSCI_A3-> BRW = 6; //baudrate of 115200
       EUSCI_A3->CTLW0 |= BIT7;
       EUSCI_A3-> MCTLW  |= (BIT0 | BIT7);
       EUSCI_A3 -> CTLW0 &= ~EUSCI_A_CTLW0_SWRST;

    NVIC_EnableIRQ(EUSCIA3_IRQn);
}


void UART_ConfigureA2(){

    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
          CS_CTL1_SELS_3 |                // SMCLK = DCO
          CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    P3->SEL0 |= (BIT2 | BIT3);
    P3->SEL1 &= ~(BIT2 | BIT3);
    EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_MODE_0; // UART Mode
    EUSCI_A2->CTLW0 &= ~(EUSCI_A_CTLW0_PEN | EUSCI_B_CTLW0_MSB | EUSCI_B_CTLW0_SEVENBIT | EUSCI_A_CTLW0_SPB); //Set No Parity, LSB First, 8-bit data, 1 start bit, 1 stop bit

    EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;

    // Baud Rate calculation
       // 12000000/(16*9600) = 78.125
       // Fractional portion = 0.125
       // User's Guide Table 21-4: UCBRSx = 0x10
       // UCBRFx = int ( (78.125-78)*16) = 2
       EUSCI_A2->BRW = 78;                     // 12000000/16/9600
       EUSCI_A2->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
               EUSCI_A_MCTLW_OS16;

       EUSCI_A2->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
       EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
       EUSCI_A2->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

   NVIC_EnableIRQ(EUSCIA2_IRQn);
}















//Testing with USB Serial connection
/***************************************************************************************************************************************/
void UART_ConfigureA0(){

    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
    CS_CTL1_SELS_3 |                // SMCLK = DCO
    CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    P1->SEL0 |= BIT2 | BIT3;
    P1->SEL1 &= ~(BIT2 | BIT3);
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_MODE_0; // UART Mode
    EUSCI_A0->CTLW0 &= ~(EUSCI_A_CTLW0_PEN | EUSCI_B_CTLW0_MSB | EUSCI_B_CTLW0_SEVENBIT | EUSCI_A_CTLW0_SPB); //Set No Parity, LSB First, 8-bit data, 1 start bit, 1 stop bit

    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;

    // Baud Rate calculation
	 // 12000000/(16*9600) = 78.125
	 // Fractional portion = 0.125
	 // User's Guide Table 21-4: UCBRSx = 0x10
	 // UCBRFx = int ( (78.125-78)*16) = 2
	 EUSCI_A0->BRW = 78;                     // 12000000/16/9600
	 EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
			 EUSCI_A_MCTLW_OS16;

	 EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
	 EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
	 EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    NVIC_EnableIRQ(EUSCIA0_IRQn);
}

void UART_send_byteA0(uint8_t data){
   while((EUSCI_A0->STATW & EUSCI_A_STATW_BUSY)); //May have to change to using the TXIFG flag
        EUSCI_A0->TXBUF = data;
}

void UART_send_A0(const uint8_t * data, uint32_t length){
    int i = 0;
    for(i=0 ; i<length ; i++){
        UART_send_byteA0(data[i]);
    }
}

