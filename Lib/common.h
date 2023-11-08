/*
 * common.h
 *
 *  Created on: Apr 24, 2023
 *      Author: duyph
 */

#ifndef LIB_COMMON_H_
#define LIB_COMMON_H_
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdbool.h>
#include<stdlib.h>
#include <math.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
//#include "../../../driver/uart.h"
//#include ".../freertos/FreeRTOS.h"

#define DAM_BUF_TX 512
#define DAM_BUF_RX 2049
#define MAX_RETRY	10
#define BUF_SIZE (2048)


#define SERVER "sanslab1.ddns.net"
#define PORT "5001"
#define ID_Device "BKRES_sensor"

#define ECHO_TEST_TXD  17
#define ECHO_TEST_RXD  16
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      2
#define ECHO_UART_BAUD_RATE     38400
#define uart_rx_task_STACK_SIZE    2048



extern bool Flag_Connected_Server ;
extern bool Flag_Wait_Exit;
extern bool Flag_Device_Ready;

extern  uint8_t ATC_Sent_TimeOut;
typedef enum
{
    EVENT_OK = 0,
    EVENT_TIMEOUT,
    EVENT_ERROR,
} SIMCOM_ResponseEvent_t;

typedef void (*SIMCOM_SendATCallBack_t)(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);
extern SIMCOM_ResponseEvent_t AT_RX_event;
typedef struct {
	char CMD[DAM_BUF_TX];
	uint32_t lenCMD;
	char ExpectResponseFromATC[20];
	uint32_t TimeoutATC;
	uint32_t CurrentTimeoutATC;
	uint8_t RetryCountATC;
	SIMCOM_SendATCallBack_t SendATCallBack;
}ATCommand_t;
extern  ATCommand_t SIMCOM_ATCommand;

typedef struct{
	float  temp;
	float pH;
	float DO;
	float EC;
}data_type;


#define AT_BUFFER_SZ 128
extern char AT_BUFFER[AT_BUFFER_SZ];
extern char *SIM_TAG ;

typedef struct{
	char * API_key;
	char *Server;
	char *Port;
}Server;




extern Server server_sanslab;
#endif /* LIB_COMMON_H_ */
