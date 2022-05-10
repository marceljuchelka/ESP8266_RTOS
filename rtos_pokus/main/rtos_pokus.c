/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
//#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_flash_partitions.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "portmacro.h"
#include "sdkconfig.h"

TaskHandle_t BlikLedHandle;
TaskHandle_t Print1Handle;
TaskHandle_t Print2Handle;

#define MB_LED	GPIO_NUM_2

//void print_time_promenna(void *arg){
//	 struct timeval now;
//	 time_t now1;
//	 localtime(&now);
//	printf("promenna ted je %lu \n",now.tv_usec );
//}

void vBlink_Led2(void *arg){
	gpio_config_t gpio_cfg;
	gpio_cfg.pin_bit_mask = GPIO_Pin_2;
	gpio_cfg.mode = GPIO_MODE_DEF_OUTPUT;
	gpio_config(&gpio_cfg);
	uint8_t uroven=0;

	while(1){
		gpio_set_level(GPIO_NUM_2, uroven=uroven^1);
		vTaskDelay(1);

	}
}

void vPrintFreeMemory(void *arg) {
	while (1) {
		ESP_LOGI("Free Mem:","%d\n", esp_get_free_heap_size());
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void vText2(void *arg) {
	ESP_LOGI("print2","created");
	while (1) {
		vTaskDelay(300 / portTICK_PERIOD_MS);
		printf("Text2 \n");
		ESP_LOGI("print2","kill");
		vTaskDelete(Print2Handle);
	}
}


void vText1(void *arg) {
	while (1) {
		printf("Text1 \n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		xTaskCreate(vText2, "text2", 2048, NULL, 1, &Print2Handle);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
//		ESP_LOGI("print2","kill");
//		vTaskDelete(Print2Handle);
	}
}


typedef struct {
	uint8_t pocitadlo;
	char	text[128];
} T_FLASH_DATA;
#define flash_data_base		0x1FC*0x1000		//adresa ulozeni dat
#define flash_sector_size	0x1000				//velikost sektoru- 4096 Byte
#define flash_addres 		flash_data_base		//

void save_data_flash(void *arg){
	esp_err_t esp_error;
	T_FLASH_DATA flashdata = {.pocitadlo = 23,.text = "Toto je test flash"};
	printf("save pocitadlo %d text %s\n ",flashdata.pocitadlo, flashdata.text);
	esp_error = spi_flash_write(flash_addres, (uint32_t*)&flashdata, sizeof(flashdata));
//	spi_flash_erase_sector(0)
	if(esp_error == ESP_OK) printf("save OK\n");
	else printf ("save error %s\n", esp_err_to_name(esp_error));
}

void read_data_flash(void *arg){
	esp_err_t esp_error;
	T_FLASH_DATA flashdata = {.pocitadlo = 0,.text = "neni text"};
	esp_error = spi_flash_read(flash_addres, (uint32_t*)&flashdata, sizeof(flashdata));
	if(esp_error == ESP_OK)
		printf("read pocitadlo %d text %s\n ",flashdata.pocitadlo, flashdata.text);
	else printf("read error %s\n", esp_err_to_name(esp_error));
}

void app_main()
{
	printf("velikost flash %d\n", spi_flash_get_chip_size());

	read_data_flash(0);
	save_data_flash(0);
	read_data_flash(0);
	printf("mazani sector 0 %s\n", esp_err_to_name(spi_flash_erase_sector(0x1FC)));
	read_data_flash(0);
	save_data_flash(0);
	read_data_flash(0);
	vTaskDelay(200);
	printf("mazani sector 0 %s\n", esp_err_to_name(spi_flash_erase_sector(0x1FC)));


	xTaskCreate(vText1, "text1", 2048, NULL, 1, &Print1Handle);
//	xTaskCreate(vText2, "text2", 512, NULL, 1, &Print2Handle);
	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 2, NULL);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1, &BlikLedHandle);


}
