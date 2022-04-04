/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "sys/_timeval.h"
#include "portmacro.h"
#include "sdkconfig.h"

TaskHandle_t task1hadle;
TaskHandle_t task2hadle;
TaskHandle_t task3hadle;

#define MB_LED	GPIO_NUM_2
void print_time_promenna(void *arg){
	 struct timeval now;
	 time_t now1;
	 localtime(&now1);
	printf("promenna ted je %lu \n",now.tv_usec );
}

void vBlink_Led2(void *arg){
	gpio_config_t gpio_cfg;
	gpio_cfg.pin_bit_mask = GPIO_Pin_2;
	gpio_cfg.mode = GPIO_MODE_DEF_OUTPUT;
	gpio_config(&gpio_cfg);
	uint8_t uroven=0;

	while(1){
		gpio_set_level(GPIO_NUM_2, uroven=uroven^1);
		vTaskDelay(20);
	}
}

void vText1(void *arg){

while(1){
	printf("Text1 \n");
	vTaskDelay(500/portTICK_PERIOD_MS);
	}
}

void vText2(void *arg){

while(1){
	printf("Text2 \n");
	vTaskDelay(1500/portTICK_PERIOD_MS);
	print_time_promenna(0);
	}
}

void vPrintFreeMemory(void *arg) {
	while (1) {
		printf("Task1: %d \n", uxTaskGetStackHighWaterMark(task1hadle));
		printf("Task2: %d \n", uxTaskGetStackHighWaterMark(task2hadle));
		printf("blikled: %d \n", uxTaskGetStackHighWaterMark(task3hadle));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
void app_main()
{
	xTaskCreate(vText1, "text1", 512, NULL, 1, &task1hadle);
	xTaskCreate(vText2, "text2", 512, NULL, 1, &task2hadle);
	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 2, NULL);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1, task3hadle);


}
