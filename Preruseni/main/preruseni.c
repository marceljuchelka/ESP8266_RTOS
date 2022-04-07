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
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "FreeRTOSConfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "portmacro.h"
#include "sdkconfig.h"

static TaskHandle_t task1hadle;
static TaskHandle_t task2hadle;
static TaskHandle_t task3hadle;
static TaskHandle_t IRDAtest_handle;
SemaphoreHandle_t	xIrda_semafor;
QueueHandle_t xIrdaQueue;

BaseType_t xHigherPriorityTaskWoken;

#define MB_LED	GPIO_NUM_2
#define IRDA_pin	GPIO_Pin_12


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
		vTaskDelay(20);
	}
}



void vText2(void *arg){
while(1){
	printf("Text2 \n");
	vTaskDelay(1500/portTICK_PERIOD_MS);
	}
}

void vPrintFreeMemory(void *arg) {
	while (1) {
		printf("Task1: %d \n", uxTaskGetStackHighWaterMark(task1hadle));
		printf("Task2: %d \n", uxTaskGetStackHighWaterMark(task2hadle));
//		printf("blikled: %d \n", uxTaskGetStackHighWaterMark(task3hadle));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

//typedef struct{
//	uint8_t	poradi;
//	uint32_t cas;
//} T_IRDA_fronta;

/*preruseni od IRDA */
static void isr_handler_IRDA(void *arg) {
static	int64_t cas_h = 0, cas_l = 0;
		int64_t vysledek = 0;
//	xTaskResumeFromISR(IRDAtest_handle);

	if (!gpio_get_level(GPIO_NUM_12)) {
		cas_h = esp_timer_get_time();
		if((cas_h - cas_l) > 12000) cas_l = 0;
		vysledek = cas_h - cas_l;
		xQueueSendToBackFromISR(xIrdaQueue,&vysledek,NULL);
		gpio_set_level(GPIO_NUM_16, 1);
		gpio_set_level(GPIO_NUM_2, 0);
	} else {
		cas_l = esp_timer_get_time();
		vysledek = cas_l - cas_h;
		if(vysledek>9000 && vysledek < 9200) {
			xQueueSendToBackFromISR(xIrdaQueue,&vysledek,NULL);
		}
		gpio_set_level(GPIO_NUM_16, 0);
		gpio_set_level(GPIO_NUM_2, 1);
	}
}

static void test_IRDA(void *arg) {
	int64_t	cas_prijaty;
	BaseType_t xStatus = 0;
	uint8_t pocitadlo = 0;
	printf("----------> testIRDA\n");
	while (1) {
		xStatus = xQueueReceive(xIrdaQueue, &cas_prijaty, 100);
		if (xStatus == pdPASS) {
			printf("pocet ve fronte %d hodnota %lld \n", pocitadlo++, cas_prijaty);

		}
		else pocitadlo = 0;
	}
}

void vText1(void *arg){
while(1){
	printf("%lld \n",esp_timer_get_time());
	printf("Text1 \n");
//	vTaskResume(IRDAtest_handle);
	vTaskDelay(500/portTICK_PERIOD_MS);
	}
}

void app_main()
{
	xIrda_semafor = xSemaphoreCreateCounting(40,0);
	xIrdaQueue = xQueueCreate(40,sizeof(int64_t));

	gpio_config_t cfg;
	cfg.pin_bit_mask	=	IRDA_pin;
	cfg.mode			= 	GPIO_MODE_INPUT;
	cfg.pull_up_en		= 	GPIO_PULLUP_ENABLE;
	cfg.intr_type		= 	GPIO_INTR_ANYEDGE;
	gpio_config(&cfg);

	cfg.pin_bit_mask	=	GPIO_Pin_2 |GPIO_Pin_16;
	cfg.mode			= 	GPIO_MODE_OUTPUT;
	cfg.intr_type		=	GPIO_INTR_DISABLE;
	cfg.pull_up_en		= 	GPIO_PULLUP_ENABLE;\
	cfg.pull_down_en	= 	GPIO_PULLDOWN_DISABLE;
	gpio_config(&cfg);

	printf("start\n");
	printf("*** preruseni ***");
	vTaskDelay(100);


	gpio_install_isr_service(0);
//	if(gpio_isr_handler_add(GPIO_NUM_14, isr_handler_IRDA, NULL) == ESP_OK) printf("handler OK\n");
//	else printf("handler chyba\n");
	gpio_isr_handler_add(GPIO_NUM_12, isr_handler_IRDA, NULL);
	printf("pred task\n");

//	xQueueReset(xIrdaQueue);


//	xTaskCreate(vText1, "text1", 2048, NULL, 1, &task1hadle);
//	xTaskCreate(vText2, "text2", 512, NULL, 1, &task2hadle);
//	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 2, NULL);
//	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1, task3hadle);
	xTaskCreate(test_IRDA, "test_IRDA", 2048, NULL, 1, &IRDAtest_handle);


	while(1){
		vTaskDelay(100);
//		printf("volna pamet %d \n", esp_get_free_heap_size());
//		printf("hodnota casu %lld \n",esp_timer_get_time());
	}


}
