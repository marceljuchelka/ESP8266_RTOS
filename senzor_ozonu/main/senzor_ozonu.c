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
#include "../components/ADS_1115/ads_1115.h"
//#include "components/ULP/ulp.h"
#define MB_LED	GPIO_NUM_2

TaskHandle_t	BlikLedMBHandle;
TaskHandle_t	PrintFreeMemoryHandle;


void vBlink_Led2(void *arg){
	gpio_config_t gpio_cfg;
	gpio_cfg.pin_bit_mask = (1<<MB_LED);
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
	uint8_t MUX = 1;
	while (1) {
		printf("BlikLedMBHandle: %d \n", uxTaskGetStackHighWaterMark(BlikLedMBHandle));
		printf("vPrintFreeMemory: %d \n", uxTaskGetStackHighWaterMark(NULL));
		ads_init();
//		printf("MUX %d   hodnota: %f \n", MUX, ads_U_input_single(MUX));
//		if(++MUX == 5)MUX = 1;
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}



void app_main()
{
	printf("start\n");
	printf("*** senzor ozonu ***\n");
	vTaskDelay(100);
	my_i2c_config();

	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 1, &PrintFreeMemoryHandle);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1,&BlikLedMBHandle );



}
