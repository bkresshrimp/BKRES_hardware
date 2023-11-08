/*
 * SDCard.c
 *
 *  Created on: Apr 26, 2023
 *      Author: duyph
 */

#include "../Lib/SDCard.h"
spi_bus_config_t bus_spi_cfg={
	.mosi_io_num = PIN_NUM_MOSI,
	.miso_io_num = PIN_NUM_MISO,
	.sclk_io_num = PIN_SCK,
	.quadhd_io_num = -1,
	.quadwp_io_num = -1,
	.max_transfer_sz = 4000,
};
esp_vfs_fat_mount_config_t mount_config={
		.format_if_mount_failed = false,
		.max_files = 10,
		.allocation_unit_size= 16*1024
};
sdmmc_card_t *card;
const char mount_point[] = MOUT_POINT;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();

esp_err_t sd_card_intialize(){
	esp_err_t err;
	ESP_LOGI(__func__, "Initializing SD card");

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.
	ESP_LOGI(__func__, "Using SPI peripheral");
	err = spi_bus_initialize(host.slot, &bus_spi_cfg, SPI_DMA_CH_AUTO);

	if(err != ESP_OK){
		ESP_LOGE(__func__, "Failed to initialize bus.");
		ESP_LOGE(__func__, "Failed to initialize the SDcard.");
		return ESP_FAIL;
	}
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;

	ESP_LOGI(__func__, "Mounting filesystem");
	err = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,&mount_config ,&card);
	if (err != ESP_OK) {
		if (err == ESP_FAIL) {
			ESP_LOGE(__func__, "Failed to mount filesystem. "
					"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(__func__, "Failed to initialize the card (%s). "
					"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(err));
		}
		return err;
	}
	ESP_LOGI(__func__, "SDCard has been initialized.");
	ESP_LOGI(__func__, "Filesystem mounted");
	ESP_LOGI(__func__, "SDCard properties.");
	sdmmc_card_print_info(stdout, card);
	return ESP_OK;
}

esp_err_t sd_write_file(const char *nameFile, char *format,...){

    char pathFile[64];
    sprintf(pathFile, "%s/%s.txt", mount_point, nameFile);

    ESP_LOGI(__func__, "Opening file %s...", pathFile);
    FILE *file = fopen(pathFile, "a+");
    if (file == NULL)
    {
        ESP_LOGE(__func__, "Failed to open file for writing.");
        return ESP_FAIL;
    }

    char *dataString;
    int lenght;
    va_list argumentsList;
    va_list argumentsList_copy;
    va_start(argumentsList, format);
    va_copy(argumentsList_copy, argumentsList);
    lenght = vsnprintf(NULL, 0, format, argumentsList_copy);
    va_end(argumentsList_copy);

    dataString = (char*)malloc(++lenght);
    if(dataString == NULL) {
        ESP_LOGE(SD_TAG, "Failed to create string data for writing.");
        va_end(argumentsList);
        return ESP_FAIL;
    }

    vsnprintf(dataString, (++lenght), format, argumentsList);
    ESP_LOGI(SD_TAG, "Success to create string data(%d) for writing.", lenght);
    ESP_LOGI(SD_TAG, "Writing data to file %s...", pathFile);
    ESP_LOGI(SD_TAG, "%s;\n", dataString);

    int returnValue = 0;
    returnValue = fprintf(file, "%s", dataString);
    if (returnValue < 0)
    {
        ESP_LOGE(__func__, "Failed to write data to file %s.", pathFile);
        return ESP_FAIL;
    }
    ESP_LOGI(__func__, "Success to write data to file %s.", pathFile);
    fclose(file);
    va_end(argumentsList);
    free(dataString);
    return ESP_OK;
}
esp_err_t sd_read_file(const char *nameFile,const char *format,...){
	 char pathFile[64];
	    sprintf(pathFile, "%s/%s.txt", mount_point, nameFile);

	    ESP_LOGI(__func__, "Opening file %s...", pathFile);
	    FILE *file = fopen(pathFile, "r");
	    if (file == NULL)
	    {
	        ESP_LOGE(SD_TAG, "Failed to open file for reading.");
	        return ESP_FAIL;
	    }

	    // Read a string data from file
	    char dataStr[256];
	    char *returnPtr;
	    returnPtr = fgets(dataStr, sizeof(dataStr), file);
	    fclose(file);

	    if (returnPtr == NULL)
	    {
	        ESP_LOGE(__func__, "Failed to read data from file %s.", pathFile);
	        return ESP_FAIL;
	    }

	    va_list argumentsList;
	    va_start(argumentsList, format);
	    int returnValue = 0;
	    returnValue = vsscanf(dataStr, format, argumentsList);
	    va_end(argumentsList);

	    if (returnValue < 0)
	    {
	        ESP_LOGE(__func__, "Failed to read data from file %s.", pathFile);
	        return ESP_FAIL;
	    }

	    return ESP_OK;
}
esp_err_t sd_rename_file(const char *oldNameFile, char *newNameFile){
	// Check if destination file exists before renaming
	    struct stat st;
	    if (stat(newNameFile, &st) == 0) {
	        ESP_LOGE(__func__, "File \"%s\" exists.", newNameFile);
	        return ESP_FAIL;
	    }

	    // Rename original file
	    ESP_LOGI(__func__, "Renaming file %s to %s", oldNameFile, newNameFile);
	    if (rename(oldNameFile, newNameFile) != 0)
	    {
	        ESP_LOGE(__func__, "Rename failed");
	        return ESP_FAIL;
	    } else {
	        ESP_LOGI(__func__, "Rename successful");
	        return ESP_OK;
	    }
}
esp_err_t sd_deinitialize(){
	ESP_LOGI(__func__, "Deinitializing SD card...");
	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_vfs_fat_sdcard_unmount(mount_point,card));
	ESP_LOGI(__func__, "Card unmounted.");
	 ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_free(host.slot));
	 return ESP_OK;
}
