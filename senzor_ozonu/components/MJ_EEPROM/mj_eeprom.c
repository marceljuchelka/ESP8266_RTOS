/*
 * mj_eeprom.c
 *
 *  Created on: 4. 6. 2022
 *      Author: marcel
 */



#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_spi_flash.h"
#include "mj_eeprom.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"

T_DATA_STORAGE_FLASH data_storage;

//	T_DATA_STORAGE_FLASH data_storage;
//	read_data_flash(&data_storage);
//	printf("SSID %s  PSW %s\n", data_storage.wifi_flash.ssid_actual, data_storage.wifi_flash.psw_actual);
//
//	strcpy(data_storage.wifi_flash.ssid_actual, TEST_SSID);
//	strcpy(data_storage.wifi_flash.psw_actual, TEST_PASS);
//	save_data_flash(&data_storage);
//	printf(" ram SSID %s  PSW %s\n", data_storage.wifi_flash.ssid_actual, data_storage.wifi_flash.psw_actual);
//	spi_flash_erase_sector(0x1FC);


/* ulozeni dat do flash  */
esp_err_t read_data_flash(T_DATA_STORAGE_FLASH *data, uint8_t sector){
	const char *TAG = "Read Flash";
	esp_err_t error;
	error = spi_flash_read((flash_start_sector + sector) * SPI_FLASH_SEC_SIZE, data, sizeof(T_DATA_STORAGE_FLASH));
	if(error == ESP_FAIL){
		ESP_LOGE(TAG,"read error");
	}
	else ESP_LOGI(TAG,"read OK");

	return ESP_OK;

}
esp_err_t save_data_flash(T_DATA_STORAGE_FLASH *data, uint8_t sector){
	const char *TAG = "Write Flash";
	esp_err_t error;

	spi_flash_erase_sector(flash_start_sector+sector);
	error = spi_flash_write((flash_start_sector + sector) * SPI_FLASH_SEC_SIZE, (uint32_t*)data, sizeof(T_DATA_STORAGE_FLASH));
	//	vTaskDelay(10);
	if(error == ESP_FAIL){
		ESP_LOGE(TAG,"write error");
	}
	else ESP_LOGI(TAG,"write OK sektor %d", sector);
	return ESP_OK;
}



esp_err_t read_eeprom(T_DATA_STORAGE_FLASH *flash_data){
	const char *TAG = "Read EEPROM";
	T_FLASH_SET sector0;

	if(spi_flash_read(flash_start_sector * SPI_FLASH_SEC_SIZE, (uint32_t*)&sector0, sizeof(T_FLASH_SET))== ESP_FAIL){
		ESP_LOGE(TAG,"sector 0 error");
		return ESP_FAIL;
	}
	if(sector0.sector_num == 1) {
		read_data_flash(flash_data, 1);
	}
	if(sector0.sector_num == 2) {
		read_data_flash(flash_data, 2);
	}
	return ESP_OK;
}


esp_err_t write_eeprom(T_DATA_STORAGE_FLASH *flash_data){
	const char *TAG = "Write EEPROM";
	T_FLASH_SET sector_now;


	if(spi_flash_read(flash_start_sector * SPI_FLASH_SEC_SIZE, (uint32_t*)&sector_now, sizeof(T_FLASH_SET))== ESP_FAIL){
		ESP_LOGE(TAG,"read sector 0 error");
		return ESP_FAIL;

	}
	else ESP_LOGI(TAG,"read sector 0 OK cislo %zu\n", (uint32_t)sector_now.sector_num);


	if(sector_now.sector_num != 1 && sector_now.sector_num != 2) sector_now.sector_num = 1;
	printf("SECTOR.NUM = %x\n", sector_now.sector_num);

	if (sector_now.sector_num == sector1) {
		spi_flash_erase_sector(flash_start_sector + sector2);
		if (save_data_flash(flash_data, sector2) == ESP_OK) {
			sector_now.sector_num = sector2;
			spi_flash_erase_sector(flash_start_sector + sector0);
			if (spi_flash_write(
					((uint32_t) flash_start_sector + sector0) * SPI_FLASH_SEC_SIZE,
					(uint32_t*) &sector_now, sizeof(T_FLASH_SET)) == ESP_FAIL) {
				ESP_LOGE(TAG, "write sector 0 error");
				return ESP_FAIL;
			}
		}
	} else if (sector_now.sector_num == sector2) {
		spi_flash_erase_sector(flash_start_sector + sector1);
		if (save_data_flash(flash_data, sector1) == ESP_OK) {
			sector_now.sector_num = sector1;
			spi_flash_erase_sector(flash_start_sector + sector0);
			if (spi_flash_write(
					((uint32_t) flash_start_sector + sector0) * SPI_FLASH_SEC_SIZE,
					(uint32_t*) &sector_now, sizeof(T_FLASH_SET)) == ESP_FAIL) {
				ESP_LOGE(TAG, "write sector 0 error");
				return ESP_FAIL;
			}
		}
	}

	ESP_LOGI(TAG, "write sector 0 OK cislem %d\n", sector_now.sector_num);
	return ESP_OK;
}
