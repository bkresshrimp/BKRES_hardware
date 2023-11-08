/*
 * myDO.c
 *
 *  Created on: May 2, 2023
 *      Author: duyph
 */


#include "../Lib/myDO.h"
uint32_t DO_V0 = 0, DO_V100 = 0;
 float DO_a = 1, DO_b = 0;
  uint16_t DO_Table[41] = {
         14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
         11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
         9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
         7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};


void init_param_DO(nvs_handle_t *pnvs_handle){
	esp_err_t err;
	err = nvs_open("storage", NVS_READWRITE, pnvs_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		// Read
		printf("Reading DO_0_VAL from NVS ... ");
		err = read_nvs(pnvs_handle,"storage", DO_0_VAL_ADDR, (uint32_t *)&DO_V0);

		switch (err) {
		case ESP_OK:
			printf("Done\n");
			//DO_V0 /= 1000000.0;
			printf("DO_0_VAL = %ld\n", DO_V0);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}
		err = read_nvs(pnvs_handle,"storage", DO_100_VAL_ADDR, (uint32_t *)&DO_V100);
		switch (err) {
		case ESP_OK:
			printf("Done\n");
			//DO_V100 /= 1000000.0;
			printf("DO_100_VAL = %ld\n", DO_V100);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

	}
	DO_function();
}
float convert_ADC_voltage(uint32_t avgValue, float ADC_resolution,float  ADC_Vref){
	return (float)avgValue*ADC_Vref/ADC_resolution;
}
void DO_Calib(adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, nvs_handle_t nvs_handle
		, const char * space_name
		, DO_calib_t do_calib){
	uint8_t i = 50;           // lower power consumption
	esp_err_t err;
	char * key= "";
	int buf [50];
	uint32_t avgValue;
	switch (do_calib){
	case do_calib_0:
		ESP_LOGI(TAG_DO_calib,"Start calib do 0");
		key = DO_0_VAL_ADDR;
		break;
	case do_calib_100:
		ESP_LOGI(TAG_DO_calib,"Start calib do 100");
		key = DO_100_VAL_ADDR;
		break;
	default:
		key = " ";
	}
	for (i = 50; i > 0; i--)  //
	{
		buf[i] =  adc1_get_raw(ADC1_CHAN);
		//pH->_pH_4_Voltage += (analogRead(pin) * Vref) / ADC_resolution;
		vTaskDelay(1000/portTICK_PERIOD_MS);
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
	float voltage = convert_ADC_voltage(avgValue, ADC_resolution, ADC_Vref);
	printf("DO calib %s:%f",key,voltage);
	avgValue = (uint32_t)voltage * 1000;
	err = write_nvs_func(nvs_handle, space_name, key, (uint32_t)avgValue);

	ESP_LOGI(TAG_DO_calib,"%s",(err!= ESP_OK)?"Error in save to nvs!":"Calib success!");
	read_nvs(nvs_handle, space_name, key, &avgValue);
}


float get_standard_do(float temperature){
	return (DO_Table[(int)temperature] * temperature) / (int)temperature;
}
float convert_vol2mgL(float ADC_voltage,float temperature){
	float percentDO = DO_a * ADC_voltage + DO_b;
	return (percentDO * get_standard_do(temperature));
}
void DO_function(){
	printf("DO 0V  = %ld\n",DO_V0);
		printf("DO 100 = %ld\n",DO_V100);
	DO_a = 1 / ((float)DO_V100/1000000.0 - (float)DO_V0/1000000.0);
	DO_b = - (float)DO_V0/((float)DO_V100 - (float)DO_V0);
	printf("DO a = %f\n",DO_a);
	printf("DO b = %f\n",DO_b);
}
float get_DO(nvs_handle_t nvsHandle, adc1_channel_t adc_chan, float ADC_VREF, float ADC_resolution){

			uint32_t ADC_RAW[50];
			uint32_t avg_adc = 0;
			uint32_t voltage;
			for(int i = 0; i < 50; i++){
				ADC_RAW[i] =  adc1_get_raw(adc_chan);
				//voltage = esp_adc_cal_raw_to_voltage(ADC_RAW[i], adc_chars);
				//printf("adc_val: %ld\n",ADC_RAW[i]);
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


			float adc_voltage = (float)avg_adc/(ADC_resolution + 1)*(ADC_VREF/1000.0);
			float DO_val = convert_vol2mgL(adc_voltage, 25.0);
			printf("avg_val= %ld",avg_adc);
			printf("DO val %f\n",DO_val);
			return (float)DO_val;
}
