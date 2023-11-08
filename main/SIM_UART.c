/*
 * UART_SIM.c
 *
 *  Created on: Apr 24, 2023
 *      Author: duyph
 */
#include "../Lib/common.h"
#include"../Lib/SIM_UART.h"
#include "../Lib/AT_function.h"
//bool Flag_Wait_Exit = false;
//bool Flag_Connected_Server = false;
bool Flag_Connected_Server = false;
uint8_t ATC_Sent_TimeOut = 0;
char AT_BUFFER[AT_BUFFER_SZ] = "";
SIMCOM_ResponseEvent_t AT_RX_event;
Server server_sanslab ={
		.Server = SERVER,
		.Port = PORT,
		.API_key= ID_Device

};
char *SIM_TAG = "UART SIM";
void  WaitandExitLoop(bool *Flag){
	while(1)
	{
		if(*Flag == true)
		{
			*Flag = false;
			break;
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
bool check_SIM900A(void){
	ATC_SendATCommand("AT\r\n", "OK", 2000, 4, ATResponse_callback);
	WaitandExitLoop(&Flag_Wait_Exit);
	if(AT_RX_event == EVENT_OK){
		return true;
	}else{
		return false;
	}
}
void Timeout_task(void *arg){
	while (1)
	{
		if((SIMCOM_ATCommand.TimeoutATC > 0) && (SIMCOM_ATCommand.CurrentTimeoutATC < SIMCOM_ATCommand.TimeoutATC))
		{
			SIMCOM_ATCommand.CurrentTimeoutATC += TIMER_ATC_PERIOD;
			if(SIMCOM_ATCommand.CurrentTimeoutATC >= SIMCOM_ATCommand.TimeoutATC)
			{
				SIMCOM_ATCommand.CurrentTimeoutATC -= SIMCOM_ATCommand.TimeoutATC;
				if(SIMCOM_ATCommand.RetryCountATC > 0)
				{
					ESP_LOGI(SIM_TAG,"retry count %d",SIMCOM_ATCommand.RetryCountATC-1);
					SIMCOM_ATCommand.RetryCountATC--;
					RetrySendATC();
				}
				else
				{
					if(SIMCOM_ATCommand.SendATCallBack != NULL)
					{
						printf("Time out!\n");
						SIMCOM_ATCommand.TimeoutATC = 0;
						SIMCOM_ATCommand.SendATCallBack(EVENT_TIMEOUT, "@@@");
						ATC_Sent_TimeOut = 1;
					}
				}
			}
		}
		vTaskDelay(TIMER_ATC_PERIOD / portTICK_PERIOD_MS);
	}

}
void Connect_Server(void){
	if(check_SIM900A()){
		Flag_Device_Ready = true;
		ESP_LOGI(SIM_TAG,"Connected to SIM900A\r\n");
		ATC_SendATCommand("AT+CIPSHUT\r\n", "OK", 2000, 2,ATResponse_callback);
		WaitandExitLoop(&Flag_Wait_Exit);
		ATC_SendATCommand("AT+CGREG=1\r\n","OK" ,2000, 2, ATResponse_callback);
		WaitandExitLoop(&Flag_Wait_Exit);
		if(AT_RX_event == EVENT_OK){
			sprintf(AT_BUFFER ,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",server_sanslab.Server,server_sanslab.Port);

			ATC_SendATCommand(AT_BUFFER, "CONNECT OK", 10000, 3, ATResponse_callback);
			WaitandExitLoop(&Flag_Wait_Exit);
			//ATC_SendATCommand(AT_BUFFER, "ALREADY CONNECT", 10000, 3, ATResponse_callback);
			//WaitandExitLoop(&Flag_Wait_Exit);
			if(AT_RX_event == EVENT_OK){
				ESP_LOGI(SIM_TAG,"Connected to server!\n");
				Flag_Connected_Server = true;
			}else{
				ESP_LOGW(SIM_TAG,"Error on Connect to Server\n");
				Flag_Connected_Server = false;
			}
		}else{
			ESP_LOGW(SIM_TAG,"Error on %s or %s\n","AT+CIPSHUT", "AT+CREG=1");
			Flag_Connected_Server = false;
		}
	}else{
		ESP_LOGW(SIM_TAG,"Error on connect to SIM900A\r\n");
		Flag_Device_Ready = false;
		Flag_Connected_Server = false;
	}
}
bool Send_Data_to_Server(const char *Buffer_data){

	Connect_Server();

	if(AT_RX_event == EVENT_OK && Flag_Connected_Server){
		ATC_SendATCommand("AT+CIPSEND\r\n", "", 2000, 0, NULL);
		//WaitandExitLoop(&Flag_Wait_Exit);
		vTaskDelay(1000/portTICK_PERIOD_MS);
		sprintf(AT_BUFFER,"GET /api/message/update?API=%s&mess=%s  HTTP/1.1\r\nHost: %s\r\n\r\n\x1A",server_sanslab.API_key,Buffer_data,server_sanslab.Server);
		ATC_SendATCommand(AT_BUFFER, "SEND OK", 5000, 0, ATResponse_callback);
		WaitandExitLoop(&Flag_Wait_Exit);
		if(AT_RX_event == EVENT_OK){
			ESP_LOGI(SIM_TAG,"Send data success!\n");
			return true;
		}else{
			ESP_LOGW(SIM_TAG,"Error Send data!\n");
			return false;
		}
	}
	return false;
}



