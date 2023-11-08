/*
 * myDO.h
 *
 *  Created on: May 2, 2023
 *      Author: duyph
 */

#ifndef LIB_MYDO_H_
#define LIB_MYDO_H_
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
#include "nvs_interface.h"
#define DO_0_VAL_ADDR "DO_0"
#define DO_100_VAL_ADDR "DO_100"
#define TAG_DO_calib "DO calib"

typedef enum{
	do_calib_0,
	do_calib_100
}DO_calib_t;
extern  uint16_t DO_Table[41] ;

extern uint32_t DO_V0, DO_V100;
extern float DO_a, DO_b;
void init_param_DO(nvs_handle_t *pnvs_handle);
void DO_Calib(adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, nvs_handle_t nvs_handle
		, const char * space_name
		, DO_calib_t do_calib);
float get_standard_do(float temperature);
float convert_vol2mgL(float ADC_voltage,float temperature);
void DO_function();
float get_DO(nvs_handle_t nvsHandle, adc1_channel_t adc_chan, float ADC_VREF, float ADC_resolution);
#endif /* LIB_MYDO_H_ */
