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
		vTaskDelay(50);
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
//		printf("vPrintFreeMemory: %d \n", uxTaskGetStackHighWaterMark(NULL));
//		printf("MUX %d   hodnota: %f \n", MUX, ads_U_input_single(MUX));
//		printf("Baterie %f \n",ULP_Battery_check1());
//		printf("Baterie napeti    Referencni napeti    hodnota ozonu je\n");
//		printf(" %f         %f       %f\n\n\n", ads_U_input_single(ulp_Vbat_read), ads_U_input_single(ulp_Vref_read), ULP_Vgas_read_PPM());
//		printf("OZON > %f\n",ULP_Vgas_read_PPM());

		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}

void print_PPM(void *arg){
	while(1){
		vULP_PPM_read(0);
		printf("bufConfigReg: %X", Buf_Config_register);
		ULP_pins_U_global.Vref_U = ads_read_single_mux(ulp_Vref_read) * ads_fsr_table[(Buf_Config_register>>ADS_PGA0) &0X06];
		printf("vref1 %f\n", ULP_pins_U_global.Vref_U);
		ULP_pins_U_global.Vref_U = ads_read_single_mux(ulp_Vref_read) * ads_fsr_table[ADS_FSR1];
		printf("vref2 %f\n", ULP_pins_U_global.Vref_U);
		vTaskDelay(100);
	}
}

void app_main()
{
	printf("start\n");
	printf("*** senzor ozonu ***\n");
	vTaskDelay(100);

	my_i2c_config();
//	ads_init();
	ULP_init();
//	ULP_pins_U_global.Vref_U = ads_U_input_single(ulp_Vref_read);
	ULP_set_cont(0);

//	printf("Referencni napeti je  %f\n", ULP_pins_U_global.Vref_U);
//	printf("napeti baterie %f\n" , ads_U_input_single(ulp_Vbat_read));

//	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 1, &PrintFreeMemoryHandle);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1,&BlikLedMBHandle );
	xTaskCreate(print_PPM, "printPPM", 1500, NULL, 1, NULL);
	while(1){
	}


}
