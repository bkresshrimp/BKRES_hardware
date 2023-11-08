/*
 * EC.h
 *
 *  Created on: Jun 8, 2023
 *      Author: duyph
 */

#ifndef LIB_EC_H_
#define LIB_EC_H_
#include "nvs_interface.h"
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

#define KVALUEADDR_LOW "K_value_EC_low" //the start address of the K value stored in the EEPROM
#define KVALUEADDR_HIGH "k_value_EC_high"
#define RAWEC_1413_LOW 0.70
#define RAWEC_1413_HIGH 1.80
#define RAWEC_276_LOW 1.95
#define RAWEC_276_HIGH 3.2
#define RAWEC_1288_LOW 8
#define RAWEC_1288_HIGH 16.8

extern float  EC_ecvalue;
extern float  EC_kvalue;
extern uint32_t  EC_kvalueLow;
extern uint32_t  EC_kvalueHigh;
extern float  EC_voltage;
extern float  EC_temperature;
extern float  EC_rawEC;
typedef enum EC_callib_t{
	EC_calib_low_1413,
	EC_calib_high_2_76,
	EC_calib_hight_12_88
}EC_calib_t;

#define EC_TAG "EC_TAG"

void EC_init_param(nvs_handle_t * nvs_handle);
void EC_calibrate(nvs_handle_t nvs_handle
		, adc1_channel_t ADC1_CHAN
		, float ADC_Vref
		, float ADC_resolution
		, const char *space_name
		, float EC_resolution
		, EC_calib_t EC_calib_type
		, float temperature);
float EC_get_value(adc1_channel_t ADC_CHAN, float ADC_resolution, float v_ref, float temperature);

#endif /* LIB_EC_H_ */
