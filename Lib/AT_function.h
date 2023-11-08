/*
 * AT_function.h
 *
 *  Created on: Apr 24, 2023
 *      Author: duyph
 */

#ifndef LIB_AT_FUNCTION_H_
#define LIB_AT_FUNCTION_H_
#include "common.h"

#define TIMER_ATC_PERIOD 50



void SendATCommand(void);
void RetrySendATC();
void ATC_SendATCommand(const char * Command, char * ExectResponse, uint32_t Timeout, uint8_t RetryCount, SIMCOM_SendATCallBack_t Callback);
void ATResponse_callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer);


#endif /* LIB_AT_FUNCTION_H_ */
