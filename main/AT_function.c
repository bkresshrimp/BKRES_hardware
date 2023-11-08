/*
 * AT_function.c
 *
 *  Created on: Apr 24, 2023
 *      Author: duyph
 */
#include "../Lib/common.h"
#include "../Lib/AT_function.h"
bool Flag_Wait_Exit = false ; // flag for wait response from sim or Timeout and exit loop
bool Flag_Device_Ready = false ; // Flag for SIM ready to use, False: device not connect, check by use AT\r\n command
char *TAG_ATCommand= "AT COMMAND";
ATCommand_t SIMCOM_ATCommand;
void SendATCommand(){
	uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)SIMCOM_ATCommand.CMD, strlen(SIMCOM_ATCommand.CMD));
	ESP_LOGI(TAG_ATCommand,"Send:%s\n",SIMCOM_ATCommand.CMD);
	ESP_LOGI(TAG_ATCommand,"Packet:\n-ExpectResponseFromATC:%s\n-RetryCountATC:%d\n-TimeoutATC:%ld\n-CurrentTimeoutATC:%ld",SIMCOM_ATCommand.ExpectResponseFromATC
			,SIMCOM_ATCommand.RetryCountATC,SIMCOM_ATCommand.TimeoutATC,SIMCOM_ATCommand.CurrentTimeoutATC);
	Flag_Wait_Exit = false;
}
void ATC_SendATCommand(const char * Command, char *ExpectResponse, uint32_t timeout, uint8_t RetryCount, SIMCOM_SendATCallBack_t Callback){
	strcpy(SIMCOM_ATCommand.CMD, Command);
	SIMCOM_ATCommand.lenCMD = strlen(SIMCOM_ATCommand.CMD);
	strcpy(SIMCOM_ATCommand.ExpectResponseFromATC, ExpectResponse);
	SIMCOM_ATCommand.RetryCountATC = RetryCount;
	SIMCOM_ATCommand.SendATCallBack = Callback;
	SIMCOM_ATCommand.TimeoutATC = timeout;
	SIMCOM_ATCommand.CurrentTimeoutATC = 0;


	SendATCommand();
}
void RetrySendATC(){
	SendATCommand();
}
void ATResponse_callback(SIMCOM_ResponseEvent_t event, void *ResponseBuffer){
	AT_RX_event  = event;
	if(event == EVENT_OK){
		ESP_LOGI(TAG_ATCommand, "Device is ready to use\r\n");
		Flag_Wait_Exit = true;
		Flag_Device_Ready = true;
	}
	else if(event == EVENT_TIMEOUT){
		ESP_LOGE(TAG_ATCommand, "Timeout, Device is not ready\r\n");
		Flag_Wait_Exit = true;
	}
	else if(event == EVENT_ERROR){
		ESP_LOGE(TAG_ATCommand, "AT check Error \r\n");
		Flag_Wait_Exit = true;
	}
}


