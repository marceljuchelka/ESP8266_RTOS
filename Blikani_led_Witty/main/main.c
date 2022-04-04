/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/adc.h"




#define led_pin_mb		GPIO_Pin_2
#define led_pin_red		GPIO_Pin_15
#define led_pin_green	GPIO_Pin_12
#define led_pin_blue	GPIO_Pin_13


#define led_mb_pin		GPIO_NUM_2
#define led_red_pin		GPIO_NUM_15
#define led_green_pin	GPIO_NUM_12
#define led_blue_pin	GPIO_NUM_13


adc_config_t	adc_cfg;

void vPartTestInitialize(){
	gpio_config_t	gpio_cfg;
	gpio_cfg.pin_bit_mask = led_pin_mb | led_pin_red | led_pin_green | led_pin_blue;
	gpio_cfg.mode = GPIO_MODE_DEF_OUTPUT;
	gpio_config(&gpio_cfg);
	adc_cfg.mode = ADC_READ_TOUT_MODE;
}

//void vLedToggle( void *pvParameters){
//	static uint8_t port_masc = 0;
//	configASSERT(((uint32_t)*pvParameters) == 1);
//		for(;;) gpio_set_level(led_pin_red, port_masc^=);
//
//}

void app_main()
{

	uint8_t uroven = 0;



//	gpio_set_direction(led_mb, GPIO_MODE_OUTPUT);
	while(1){
		uroven^= 1;
		gpio_set_level(led_green_pin, uroven);
		vTaskDelay(100/portTICK_PERIOD_MS);
		gpio_set_level(led_red_pin, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
		gpio_set_level(led_blue_pin, uroven);
		vTaskDelay(300/portTICK_PERIOD_MS);
		gpio_set_level(led_mb_pin, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
	}

}
