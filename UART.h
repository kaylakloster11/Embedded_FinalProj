/*
 * UART.h
 *
 *  Created on: Dec 10, 2017
 *      Author: admin
 */

#ifndef UART_H_
#define UART_H_

#include "msp.h"
#include "FPS.h"
#include <string.h>

void UART_ConfigureA2();
void UART_ConfigureA3();
void EUSCIA2_IRQHandler();

void UART_send_byteA2(uint8_t data);
void UART_send_byteA3(uint8_t data);
void UART_send_A2(const uint8_t * data, uint32_t length);
void UART_send_A3(const uint8_t * data, uint32_t length);

void UART_read_data(uint8_t * data, int length);
void UART_read_byte(uint8_t data);

//Testing with Computer Serial connection
void UART_ConfigureA0();
void UART_send_byteA0(uint8_t data);
void UART_send_A0(const uint8_t * data, uint32_t length);

//Sender Helpers
void reverse(char s[]);
void itoa(int n, char s[]);

#endif /* UART_H_ */
