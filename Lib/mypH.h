/*
 * mypH.h
 *
 *  Created on: Apr 28, 2023
 *      Author: duyph
 */

#ifndef LIB_MYPH_H_
#define LIB_MYPH_H_

#include "nvs_flash.h"
#include "nvs.h"

#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdbool.h>
#include<stdlib.h>
#include <math.h>
#include <string.h>
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PH_4_VAL_ADDR "pH4"
#define PH_6_86_VAL_ADDR "pH6_86"
#define PH_9_VAL_ADDR "pH9"
#define TAG_pH_Calib "ph_calib"
#define TAG_pH "pH TAG"
extern uint32_t _pH_4_voltage;
extern uint32_t  _pH_6_86_voltage;
extern uint32_t _pH_9_voltage;

extern bool flag_is_get_param ;
extern bool param_is_null;
typedef enum{
	pH_CALIB_4,
	pH_CALIB_6_86,
	pH_CALIB_9
}pH_calib_t;
extern double a_6_86_4;
extern double b_6_86_4;

extern double a_9_6_86;
extern double b_9_6_86;

void init_param_pH(nvs_handle_t* nvs_hande);
void pH_function_9_6_86();
void pH_function_4_6_86();
void pH_Calib(adc1_channel_t ADC1_CHAN, float ADC_VREF, float ADC_resolution
		, nvs_handle_t nvs_handle
		, const char * space_name
		, pH_calib_t pH_calib);
float get_pH_value(uint32_t ADC_value,nvs_handle_t nvs_handle, uint32_t resolution, float V_ref);
static float convert_ADC_voltage(uint32_t ADC, uint32_t resolution, float V_ref);
float get_pH(nvs_handle_t nvsHandle,adc1_channel_t adc_chan, float ADC_VREF, float ADC_resolution);
#endif /* LIB_MYPH_H_ */
