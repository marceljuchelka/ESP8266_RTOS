/* test preruseni od GPIO
 * vyuzivam ho na nacitani IR signalu
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
//static TaskHandle_t task3hadle;
static TaskHandle_t IrDecHandle;

QueueHandle_t xIrdaQueue;
QueueHandle_t xIrRecQueue;

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
	//	xTaskResumeFromISR(task1hadle);

	if (!gpio_get_level(GPIO_NUM_12)) {
		cas_h = esp_timer_get_time();
		if((cas_h - cas_l) > 12000) cas_l = 0;
		vysledek = cas_h - cas_l;
		if(vysledek > 400 && vysledek < 1800) xQueueSendToBackFromISR(xIrdaQueue,&vysledek,NULL);
		gpio_set_level(GPIO_NUM_16, 1);
		gpio_set_level(GPIO_NUM_2, 0);
	} else {
		cas_l = esp_timer_get_time();
		vysledek = cas_l - cas_h;
		if(vysledek>8800 && vysledek < 9400) {
			xQueueSendToBackFromISR(xIrdaQueue,&vysledek,NULL);
			xTaskResumeFromISR(IrDecHandle);
		}
		gpio_set_level(GPIO_NUM_16, 0);
		gpio_set_level(GPIO_NUM_2, 1);
	}
}

static void vIrDecode(void *arg) {
	uint8_t Key_button = 0;
	int16_t cas_prijaty;
	uint32_t key_val = 0;
	uint8_t pocitadlo = 0;
	while (1) {
		if (xQueueReceive(xIrdaQueue, &cas_prijaty, 1)) {
			if (cas_prijaty > 8800 && cas_prijaty < 9400) {
				pocitadlo = 0;
				key_val = 0;
			}
			if (cas_prijaty > 400 && cas_prijaty < 750)
				key_val = (key_val + 0) << 1;
			if (cas_prijaty > 1500 && cas_prijaty < 1800)
				key_val = (key_val + 1) << 1;
//			printf("poradi: %d  cas: %d\n", pocitadlo, cas_prijaty);
			pocitadlo++;
			if (pocitadlo == 32) {
				switch (key_val) {
				case 0xff9866:		//0
					Key_button = 0;
					break;
				case 0xffa25c:		//1
					Key_button = 1;
					break;
				case 0xff629c:		//2
					Key_button = 2;
					break;
				case 0xffe21c:		//3
					Key_button = 3;
					break;
				case 0xff22dc:		//4
					Key_button = 4;
					break;
				case 0xff02fc:		//5
					Key_button = 5;
					break;
				case 0xffc23c:		//6
					Key_button = 6;
					break;
				case 0xffe01e:		//7
					Key_button = 7;
					break;
				case 0xffa856:		//8
					Key_button = 8;
					break;
				case 0xff906e:		//9
					Key_button = 9;
					break;
				case 0xff6896:		//* 	10
					Key_button = 10;
					break;
				case 0xffb04e:		//#		11
					Key_button = 11;
					break;
				case 0xff10ee:		//<		12
					Key_button = 12;
					break;
				case 0xff5aa4:		//>		13
					Key_button = 13;
					break;
				case 0xff18e6:		//^		14
					Key_button = 14;
					break;
				case 0xff4ab4:
					Key_button = 15;	//v		15
					break;
				case 0xff38c6:
					Key_button = 16;	//ok	11
					break;
				}
//				printf("hodnotaIR v hex: %x button   %d\n", key_val, Key_button);

				xQueueSendToBack(xIrRecQueue, &Key_button, 10);
			vTaskSuspend(NULL);
			}
		}
		else {
			pocitadlo = 0;
			key_val = 0;
		}
	}
}

void vText1(void *arg){
	uint8_t IrKey = 0;
	BaseType_t xStatus = 0;
while(1){
//	printf("%lld \n",esp_timer_get_time());
	xStatus = xQueueReceive(xIrRecQueue, &IrKey, 1);
	if(xStatus == pdTRUE) {
		printf("IR Tlacitko: %d \n", IrKey);
		printf("Irdekode: %d \n", uxTaskGetStackHighWaterMark(IrDecHandle));
	}
//	vTaskDelay(500/portTICK_PERIOD_MS);
	}
}

void app_main()
{

	xIrdaQueue 	= xQueueCreate(64,sizeof(int16_t));
	xIrRecQueue = xQueueCreate(5,sizeof(uint8_t));

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
	printf("*** preruseni ***\n");
	vTaskDelay(100);



	printf("pred task\n");

//	xQueueReset(xIrdaQueue);


	xTaskCreate(vText1, "text1", 2048, NULL, 1, &task1hadle);
//	xTaskCreate(vText2, "text2", 512, NULL, 1, &task2hadle);
//	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 2, NULL);
//	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1, task3hadle);
	xTaskCreate(vIrDecode, "IRDA dekodovani", 2048, NULL, 1, &IrDecHandle);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(GPIO_NUM_12, isr_handler_IRDA, NULL);

	while(1){
		vTaskDelay(100);
//		printf("volna pamet %d \n", esp_get_free_heap_size());
//		printf("hodnota casu %lld \n",esp_timer_get_time());
	}


}
