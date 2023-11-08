/*
 * myTDS.c
 *
 *  Created on: May 2, 2023
 *      Author: duyph
 */

#include "../Lib/myTDS.h"
uint32_t _kValue = 0;
float _TDS_buffer_solution_val = 0,k_value;
float EC_val = 0;
void TDS_init_param(nvs_handle_t* nvs_handle){
	esp_err_t err;
	err = nvs_open("storage", NVS_READWRITE, nvs_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		// Read
		printf("Reading K_VALUE from NVS ... ");
		err = read_nvs(*nvs_handle,"storage", K_VALUE_ADDR, (uint32_t *)&_kValue);
		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("K_VALUE = %ld\n", _kValue);
			k_value = (float)_kValue/1000.0;
			printf("K_VALUE = %f\n", k_value);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
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
void TDS_calib(nvs_handle_t nvs_handle
		,adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, const char * space_name
		, float TDSSolution
){
	uint8_t i = 50;           // lower power consumption
	esp_err_t err;
	char * key= "";
	int buf [50];
	uint32_t avgValue;
	ESP_LOGI(TAG_TDS,"Start calib TDS");
	key = K_VALUE_ADDR;
	float k_value;

	for (i = 50; i > 0; i--)  //
	{
		buf[i] = 4095- adc1_get_raw(ADC1_CHAN);
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
	float adcCompensation = 1 + (1/3.9); // 1/3.9 (11dB) attenuation.
	float vPerDiv = (TDS_VREF / 4096) * adcCompensation; // Calculate the volts per division using the VREF taking account of the chosen attenuation value.
	float voltage = avgValue * vPerDiv;
	float rawECsolution =  TDSSolution*(1.0+0.02*(TDS_TEMPERATURE - 25.0));
	k_value = rawECsolution/(133.42*voltage*voltage*voltage - 255.86*voltage*voltage + 857.39*voltage);
	k_value *= 1000.0;
	ESP_LOGI(TAG_TDS,"K value:%f",k_value);
	err = write_nvs_func(nvs_handle, space_name, key, (uint32_t)k_value);

	ESP_LOGI(TAG_TDS,"%s",(err!= ESP_OK)?"Error in save to nvs!":"Calib success!");

}
float TDS_function(float ADC_Voltage){
	return 133.42 * pow(ADC_Voltage, 3) + 255.86 * pow(ADC_Voltage, 2) + 857.39 * ADC_Voltage;
}
float convertEC_TDS_val(float ADC_Voltage, float temperature, float TDS_factor){
	float EC_val = TDS_function(ADC_Voltage)* k_value;
	return EC_val / (1 + 0.02 * (temperature - 25.0));
}
float TDS_toSalt(float ADC_val,float temperature,float TDS_factor){
	return convertEC_TDS_val(ADC_val, temperature, ADC_val)*0.01*640;
}
float convert_to_ppm(float analogReading){
	float adcCompensation = 1 + (1/3.9); // 1/3.9 (11dB) attenuation.
	float vPerDiv = (TDS_VREF / 4096) * adcCompensation; // Calculate the volts per division using the VREF taking account of the chosen attenuation value.
	float averageVoltage = analogReading * vPerDiv; // Convert the ADC reading into volts
	float compensationCoefficient=1.0+0.02*(TDS_TEMPERATURE-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
	float compensationVolatge = averageVoltage / compensationCoefficient;  //temperature compensation
	float tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * k_value; //convert voltage value to tds value
	return tdsValue;
}
float get_TDS_toSalt(adc1_channel_t adc_chan, float temperature,float TDS_factor, float ADC_resolution,float ADC_VREF){
	uint32_t ADC_RAW[50];
	uint32_t avg_adc = 0;
	uint32_t voltage;
	float salt;
	float TDS;
	for(int i = 0; i < 50; i++){
		ADC_RAW[i] = (uint32_t)ADC_resolution - adc1_get_raw(adc_chan);
		//voltage = esp_adc_cal_raw_to_voltage(ADC_RAW[i], adc_chars);
		//printf("adc_val: %ld\n",ADC_RAW[i]);
		vTaskDelay(1000/portTICK_PERIOD_MS);

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
	//				float adc_voltage = (float)avg_adc/(ADC_resolution + 1)*(ADC_VREF/1000.0)*(5.0/3.3);
	//				TDS = convertEC_TDS_val(adc_voltage, temperature, TDS_factor);
	TDS = convert_to_ppm(avg_adc);

	return TDS;

}
