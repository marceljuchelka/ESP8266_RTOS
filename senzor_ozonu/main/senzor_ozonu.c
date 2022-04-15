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
#include "../components/ULP/ulp.h"
#include "../components/TM_1637_LED/tm_1637_led.h"

#define MB_LED	GPIO_NUM_2

TaskHandle_t	BlikLedMBHandle;
TaskHandle_t	PrintFreeMemoryHandle;
QueueHandle_t	OzonHandle;



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
	uint8_t MUX = 0;
	while (1) {
//		printf("BlikLedMBHandle: %d \n", uxTaskGetStackHighWaterMark(BlikLedMBHandle));
		printf("volna pamet %d\n", esp_get_free_heap_size());
		printf("vPrintFreeMemory: %d \n", uxTaskGetStackHighWaterMark(NULL));
//		printf("MUX %d   hodnota: %f \n", MUX, ads_U_input_single(MUX));
//		printf("Baterie %f \n",ULP_Battery_check1());
//		printf("Baterie napeti    Referencni napeti    hodnota ozonu je\n");
//		printf(" %f         %f       %f\n\n\n", ads_U_input_single(ulp_Vbat_read), ads_U_input_single(ulp_Vref_read), ULP_Vgas_read_PPM());
//		printf("OZON > %f\n",ULP_Vgas_read_PPM());

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void print_PPM(void *arg){
	while(1){
		while(1){
			vULP_PPM_read(0);
		vTaskDelay(200);
		}
	}
}

void vPrintOzonNaLED(void *arg) {
	float ozonPPM = 0;
	char OzonBuf[5];
	while (1) {
		if (xQueueReceive(OzonHandle, &ozonPPM, 10) == pdTRUE) printf("Hodnota ozonu> %f\n", ozonPPM);
//		if (ozonPPM<10)
			sprintf(OzonBuf,"p%d\n", (uint8_t)(ozonPPM*10));
			printf("ozon string> %s\n", OzonBuf);
			led_print(0, OzonBuf);
		vTaskDelay(100);
	}
}

void app_main()
{
	vTaskDelay(200);
	printf("start\n");
	printf("*** senzor ozonu ***\n");
	fronta_vzorku_napeti = xQueueCreate(5,sizeof(float));
	OzonHandle = xQueueCreate(1,sizeof(float));
	my_i2c_config();
	tm_1637_gpio_init();
	//	ads_init();
	ULP_init();
//	ULP_pins_U_global.Vref_U = ads_U_input_single(ulp_Vref_read);
	vULP_set_cont(0);
	led_print(0, "STRT");
	vTaskDelay(200);
	//	printf("Referencni napeti je  %f\n", ULP_pins_U_global.Vref_U);
//	printf("napeti baterie %f\n" , ads_U_input_single(ulp_Vbat_read));



	/*spusteni tasku  */
	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 1, &PrintFreeMemoryHandle);
//	xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)
	xTaskCreate(vPrintOzonNaLED, "print ozon", 2048, NULL, 1, NULL);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1,&BlikLedMBHandle );
	xTaskCreate(vULP_VoltageRead, "voltage read", 1500, NULL, 1, voltagereadHandle);
	xTaskCreate(vULP_PPM_read, "PPM read", 1500, NULL, 1, &PPMReadHandle);
	while(1){
//		ESP_LOGI("Main"," while");
	}


}
