/*
 * nvs_interface.c
 *
 *  Created on: Apr 28, 2023
 *      Author: duyph
 */



#include "../Lib/nvs_interface.h"

esp_err_t err;
esp_err_t read_nvs(nvs_handle_t nvs_handle, const char * space_name, const char * key, uint32_t* value){
	err = nvs_open(space_name, NVS_READWRITE, &nvs_handle);
		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			printf("Done\n");
		}

		err = nvs_get_i32(nvs_handle, key,(int32_t*)value);
		switch (err){
		case ESP_OK:
			ESP_LOGI(NVS_TAG, "Done read key:%s", key);
			ESP_LOGI(NVS_TAG, "%s= %ld",key,*value);

			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(NVS_TAG,"The value is not initialized yet!\n");
			break;
		default:
			ESP_LOGE(NVS_TAG, "Error (%s) reading!\n", esp_err_to_name(err));
			break;
		}
		nvs_commit(nvs_handle);
		nvs_close(nvs_handle);
		return err;
}


esp_err_t write_nvs_func(nvs_handle_t nvs_handle, const char * space_name, const char * key, uint32_t value_storage){
	err = nvs_open(space_name, NVS_READWRITE, &nvs_handle);
			if (err != ESP_OK) {
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				return err;
			} else {
				printf("Done\n");

			}
	//ESP_LOGI(NVS_TAG,"Updating restart counter in NVS ...");
	err = nvs_set_i32(nvs_handle, key, value_storage);
	printf("%s",(err!= ESP_OK)?"Failed!\n" : "Done set\n");

	if(err != ESP_OK) return err;

	printf("Committing update in NVS ....");
	err = nvs_commit(nvs_handle);
	printf("%s", (err != ESP_OK) ? "Failed!\n" : "Done Commit\n");
	return err;
}
