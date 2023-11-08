/*
 * myTDS.h
 *
 *  Created on: May 2, 2023
 *      Author: duyph
 */

#ifndef LIB_MYTDS_H_
#define LIB_MYTDS_H_
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
#include "../Lib/nvs_interface.h"
#define TAG_TDS "Tag TDS"
#define K_VALUE_ADDR "K_val_addr"
#define TDS_buffer_solution_addr "tds_buf_addr"
extern uint32_t _kValue;
extern float _TDS_buffer_solution_val, k_value;
extern float EC_val;


#define TDS_STABILISATION_DELAY     10  //(int) How long to wait (in seconds) after enabling sensor before taking a reading
#define TDS_NUM_SAMPLES             10  //(int) Number of reading to take for an average
#define TDS_SAMPLE_PERIOD           20  //(int) Sample period (delay between samples == sample period / number of readings)
#define TDS_TEMPERATURE             29.3  //(float) Temperature of water (we should measure this with a sensor to get an accurate reading)
#define TDS_VREF                    1.18   //(float) Voltage reference for ADC. We should measure the actual value of each ESP32

void TDS_init_param(nvs_handle_t* nvs_handle);

void TDS_calib(nvs_handle_t nvs_handle
		,adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, const char * space_name
		, float TDSSolution
		);
float TDS_function(float ADC_voltage);
float convertEC_TDS_val(float ADC_Voltage, float temperature,float TDS_factor);
float TDS_toSalt(float ADC_val,float temperature,float TDS_factor);
float get_TDS_toSalt(adc1_channel_t adc_chan, float temperature,float TDS_factor, float ADC_resolution,float ADC_VREF);
#endif /* LIB_MYTDS_H_ */
