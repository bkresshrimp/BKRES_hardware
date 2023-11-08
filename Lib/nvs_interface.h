/*
 * nvs_interface.h
 *
 *  Created on: Apr 28, 2023
 *      Author: duyph
 */

#ifndef LIB_NVS_INTERFACE_H_
#define LIB_NVS_INTERFACE_H_

#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nvs_flash.h"
#include "nvs.h"

#define NVS_TAG "nvs interface"
esp_err_t read_nvs(nvs_handle_t nvs_handle
		, const char * space_name
		, const char * key
		, uint32_t *value);
esp_err_t write_nvs_func(
		 nvs_handle_t nvs_handle
		, const char * space_name
		, const char *key
		, uint32_t value_storage);


#endif /* LIB_NVS_INTERFACE_H_ */
