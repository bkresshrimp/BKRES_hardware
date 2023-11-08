/*
 * EC.c
 *
 *  Created on: Jun 8, 2023
 *      Author: duyph
 */

#include "../Lib/EC.h"
#include "../Lib/nvs_interface.h"

#define RES2 (7500.0/0.66)
#define ECREF 20.0

uint32_t  EC_kvalueLow;
uint32_t  EC_kvalueHigh;

void EC_init_param(nvs_handle_t * nvs_handle){
	esp_err_t err = nvs_open("storage", NVS_READWRITE, nvs_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		// Read
		printf("Reading EC_kvalue from NVS ... ");
		err = read_nvs(nvs_handle,"storage", KVALUEADDR_LOW, (uint32_t *)&EC_kvalueLow);

		switch (err) {
		case ESP_OK:
			printf("Done\n");

			printf("KVALUEADDR_LOW = %ld\n", EC_kvalueLow);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			EC_kvalueLow = 1000;
			write_nvs_func(nvs_handle, "storage", KVALUEADDR_LOW, (uint32_t)EC_kvalueLow);
			EC_kvalueLow = 1;
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}
		err = read_nvs(nvs_handle,"storage", KVALUEADDR_HIGH, (uint32_t *)&EC_kvalueHigh);
		switch (err) {
		case ESP_OK:
			printf("Done\n");

			printf("EC_kvalueHigh = %ld\n", EC_kvalueHigh);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			EC_kvalueHigh = 1000;
			write_nvs_func(nvs_handle, "storage", KVALUEADDR_HIGH, (uint32_t)EC_kvalueHigh);
			EC_kvalueHigh = 1;
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}
	}
}
static float convert_ADC_voltage(uint32_t avgValue, float ADC_resolution,float  ADC_Vref){
	return (float)avgValue*ADC_Vref/ADC_resolution;
}
void EC_calibrate(nvs_handle_t nvs_handle
		, adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, const char *space_name
		, float EC_resolution
		, EC_calib_t EC_calib_type
		, float temperature){
	uint8_t sampling = 50;
	esp_err_t err;
	char *key="";
	int buf[50];
	float EC_solution = 0.0;
	uint32_t avgValue;
	switch(EC_calib_type){
	case EC_calib_low_1413:
		ESP_LOGI(EC_TAG,"Start calib EC_calib_low_1413");
		EC_solution = 1.413;
		key = KVALUEADDR_LOW;
		break;
	case EC_calib_high_2_76:
		ESP_LOGI(EC_TAG,"Start calib EC_calib_high_2_76");
		EC_solution = 2.76;
		key = KVALUEADDR_HIGH;
		break;
	case EC_calib_hight_12_88:
		ESP_LOGI(EC_TAG,"Start calib EC_calib_hight_12_88");
		EC_solution = 12.88;
		key = KVALUEADDR_HIGH;
		break;
	}

	for (int i = 50; i > 0; i--)  //
	{
		buf[i] =   adc1_get_raw(ADC1_CHAN);
		//pH->_pH_4_Voltage += (analogRead(pin) * Vref) / ADC_resolution;
		vTaskDelay(200/portTICK_PERIOD_MS);
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
	float compECsolution = EC_solution* (1.0 + 0.0185 * (temperature - 25.0));
	float voltage = convert_ADC_voltage(avgValue,ADC_resolution,ADC_Vref);
	float KValueTemp = RES2 * ECREF * compECsolution / 1000.0 / voltage/10.0;
	KValueTemp *= 1000.0;
	err = write_nvs_func(nvs_handle, space_name, key, (uint32_t)KValueTemp);

	ESP_LOGI(EC_TAG,"%s",(err!= ESP_OK)?"Error in save to nvs!":"Calib success!");
	read_nvs(nvs_handle, space_name, key, &KValueTemp);
}
/*
 * Function get EC value
 * @param v_ref: 3300
 * return float
 * */
float EC_get_value(adc1_channel_t ADC_CHAN, float ADC_resolution, float v_ref, float temperature){

	uint32_t ADC_RAW[50];
	uint32_t avg_adc = 0;

	for(int i = 0; i < 50; i++){
		ADC_RAW[i] =  adc1_get_raw(ADC_CHAN);
		//voltage = esp_adc_cal_raw_to_voltage(ADC_RAW[i], adc_chars);
		//printf("adc_val: %ld\n",ADC_RAW[i]);
		//vTaskDelay(300/portTICK_PERIOD_MS);

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

	float voltage = convert_ADC_voltage(avg_adc, ADC_resolution, v_ref);

	float value = (((voltage/RES2)/ECREF)*10.0);
	 value = value * (float)EC_kvalueHigh;
	value = value/(1.0 + 0.0185*(temperature - 25.0));
	printf("\nEC_value %f\n",value);
	return value;
}
