/*
 * SIM_UART.h
 *
 *  Created on: Apr 24, 2023
 *      Author: duyph
 */

#ifndef LIB_SIM_UART_H_
#define LIB_SIM_UART_H_
//char AT_BUFFER[AT_BUFFER_SZ];



void  WaitandExitLoop(bool *Flag);
bool check_SIM900A(void);
void Timeout_task(void *arg);
bool Send_Data_to_Server(const char *Buffer_data);
void Connect_Server(void );

#endif /* LIB_SIM_UART_H_ */
