/*
 * SDCard.h
 *
 *  Created on: Apr 25, 2023
 *      Author: duyph
 */

#ifndef LIB_SDCARD_H_
#define LIB_SDCARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_log.h>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_SCK 18
#define PIN_NUM_CS 5


#define MOUT_POINT "/sdcard"
#define MAX_CHAR_SIZE 128


static const char * SD_TAG = "SD Card";
extern spi_bus_config_t bus_spi_cfg;
extern esp_vfs_fat_mount_config_t mount_config;
extern sdmmc_card_t *card;
extern const char mount_point[20];
extern sdmmc_host_t host;
extern sdspi_device_config_t slot_config ;
esp_err_t sd_card_intialize();
esp_err_t sd_write_file(const char *nameFile, char *format,...);
esp_err_t sd_read_file(const char *nameFile,const char *format,...);
esp_err_t sd_rename_file();
esp_err_t sd_deinitialize();


#endif /* LIB_SDCARD_H_ */
