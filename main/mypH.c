/*
 * mypH.c
 *
 *  Created on: Apr 28, 2023
 *      Author: duyph
 */
#include "../Lib/mypH.h"

#include "../Lib/nvs_interface.h"
#include <stdbool.h>

uint32_t _pH_4_voltage = 0;
uint32_t  _pH_6_86_voltage = 0;
uint32_t _pH_9_voltage  = 0;

bool flag_is_get_param = false;

bool param_is_null = true ;

double a_6_86_4 = 1.0;
double b_6_86_4 = 0.0;

double a_9_6_86 = 1.0 ;
double b_9_6_86 = 0.0;

void init_param_pH(nvs_handle_t * nvsHandle){
	esp_err_t err;
	err = nvs_open("storage", NVS_READWRITE, nvsHandle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		// Read
		printf("Reading _pH_4_voltage from NVS ... ");
		err = read_nvs(nvsHandle,"storage",PH_4_VAL_ADDR, (uint32_t *)&_pH_4_voltage);

		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("_pH_4_voltage = %ld\n", _pH_4_voltage);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}
		err = read_nvs(nvsHandle, "storage",PH_6_86_VAL_ADDR, (uint32_t *)&_pH_6_86_voltage);
		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("_pH_6_86_voltage = %ld\n", _pH_6_86_voltage);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}
		err = read_nvs(nvsHandle,"storage", PH_9_VAL_ADDR,(uint32_t *) &_pH_9_voltage);
		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("_pH_9_voltage = %ld\n", _pH_9_voltage);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			flag_is_get_param = false;
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
			flag_is_get_param = false;
		}
		flag_is_get_param = true;
		pH_function_9_6_86();
		pH_function_4_6_86();
	}
}

void pH_function_9_6_86(){
	a_9_6_86 = (9.0 - 6.86) / ((float)_pH_9_voltage/1000000.0 - (float)_pH_6_86_voltage/1000000.0);
	b_9_6_86 = 6.86 - (a_9_6_86 *(float)_pH_6_86_voltage/1000000.0);
	printf("a_9_6_86: %f \t b_9_6_86: %f \n",(float)a_9_6_86,(float)b_9_6_86);
}
void pH_function_4_6_86(){
	a_6_86_4 = (6.86 - 4.0) / ((float)_pH_6_86_voltage/1000000.0 - (float)_pH_4_voltage/1000000.0);
	b_6_86_4 = 4 - (a_6_86_4 *(float)_pH_4_voltage/1000000.0);
	printf("a_6_86_4: %f \t b_6_86_4: %f\n ",(float)a_6_86_4,(float)b_6_86_4);
}
void pH_Calib(adc1_channel_t ADC1_CHAN, float ADC_VREF, float ADC_resolution
		, nvs_handle_t nvsHandle
		, const char * space_name
		, pH_calib_t pH_calib_t){
	uint8_t i = 50;           // lower power consumption
	esp_err_t err;
	char * key= "";
	int buf [50];
	uint32_t avgValue;
	switch (pH_calib_t){
	case pH_CALIB_4:
		ESP_LOGI(TAG_pH_Calib,"Start calib pH 4");
		key = PH_4_VAL_ADDR;
		break;
	case pH_CALIB_6_86:
		ESP_LOGI(TAG_pH_Calib,"Start calib pH 6.86");
		key = PH_6_86_VAL_ADDR;
		break;
	case pH_CALIB_9:
		ESP_LOGI(TAG_pH_Calib,"Start calib pH 9");
		key = PH_9_VAL_ADDR;
		break;
	}
	for (i = 50; i > 0; i--)  //
	{
		buf[i] = adc1_get_raw(ADC1_CHAN);
		//ESP_LOGI(TAG_pH,"ADC value : %d",(int)buf[i]);
		//pH->_pH_4_Voltage += (analogRead(pin) * Vref) / ADC_resolution;
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

	for(int i=0;i<50;i++)        //sort the analog from small to large
	{
		for(int j=i+1;j<50;j++)
		{
			if(buf[i]>buf[j])
			{
				int temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	avgValue = 0;
	for(int i = 10; i < 40; i ++){
		avgValue +=buf[i];
	}
	avgValue /= 30;
	ESP_LOGI(TAG_pH,"avgVal ADC: %d",(int)avgValue);
	float voltage = convert_ADC_voltage(avgValue, ADC_resolution, ADC_VREF);
	avgValue = (uint32_t)voltage * 1000;
	ESP_LOGI(TAG_pH,"Saving param to NVS: %d",(int)avgValue);
	printf("key:%s\n",key);
	err = write_nvs_func(nvsHandle, space_name, key, (uint32_t)avgValue);
	ESP_LOGI(TAG_pH_Calib,"%s",esp_err_to_name(err));
	ESP_LOGI(TAG_pH_Calib,"%s",(err!= ESP_OK)?"Error in save to nvs!":"Calib success!");
	read_nvs(nvsHandle, space_name, key, &_pH_4_voltage);
}

float get_pH_value(uint32_t ADC_value,nvs_handle_t nvsHandle, uint32_t resolution, float V_ref){
	if(flag_is_get_param == false){
		ESP_LOGI(TAG_pH,"Reading param in EEPROM....");
		init_param_pH(nvsHandle);
		if(_pH_4_voltage == 0 || _pH_6_86_voltage == 0 || _pH_9_voltage == 0){
			ESP_LOGI(TAG_pH,"No param in EEPROM!");
			_pH_4_voltage = 0;
			_pH_6_86_voltage = 0;
			_pH_9_voltage = 0;
			return convert_ADC_voltage(ADC_value, resolution, V_ref);
		}

	}

	float voltage = convert_ADC_voltage(ADC_value, resolution, V_ref);
	//printf("voltage ph adc: %f\n",voltage);
	return (voltage/1000.0 <(float)_pH_6_86_voltage/1000000.0?(a_6_86_4*voltage/1000.0 + b_6_86_4)
			:(a_9_6_86*voltage/1000.0 +b_9_6_86));



}
float convert_ADC_voltage(uint32_t ADC, uint32_t resolution, float V_ref){
	return (float)ADC/(float)resolution * V_ref;
}

float get_pH(nvs_handle_t nvsHandle,adc1_channel_t adc_chan, float ADC_VREF, float ADC_resolution){
	uint32_t ADC_RAW[50];
	uint32_t avg_adc = 0;

	float pH_val;
	for(int i = 0; i < 50; i++){
		ADC_RAW[i] =  adc1_get_raw(adc_chan);
		//voltage = esp_adc_cal_raw_to_voltage(ADC_RAW[i], adc_chars);
//		printf("adc_val: %ld\n",ADC_RAW[i]);
		//vTaskDelay(200/portTICK_PERIOD_MS);

	}
	for(int i = 0; i < 50; i ++){
		for(int j = i; i < 50; i ++){
			if(ADC_RAW[j]< ADC_RAW[i]){
				uint32_t tmp = ADC_RAW[i];
				ADC_RAW[i]= ADC_RAW[j];
				ADC_RAW[j] = tmp;
			}
		}
	}

	for(int i = 10; i < 40; i ++){
		avg_adc += ADC_RAW[i];
	}
	avg_adc /=30;

	pH_val = get_pH_value(avg_adc, nvsHandle,ADC_resolution, ADC_VREF);
	return pH_val;
}
