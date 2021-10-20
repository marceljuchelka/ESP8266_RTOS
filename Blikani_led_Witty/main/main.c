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
#include "driver/uart_select.h"
#include "driver/uart.h"
#include "portmacro.h"

#include "FreeRTOSConfig.h"
#include "..\build\include\sdkconfig.h"

#define led_pin_mb		GPIO_Pin_2
#define led_pin_red		GPIO_Pin_15
#define led_pin_green	GPIO_Pin_12
#define led_pin_blue	GPIO_Pin_13


#define led_mb		GPIO_NUM_2
#define led_red		GPIO_NUM_15
#define led_green	GPIO_NUM_12
#define led_blue	GPIO_NUM_13

gpio_config_t	gpio_cfg;




void app_main()
{

	uint8_t uroven = 0;
	gpio_cfg.pin_bit_mask = led_pin_mb | led_pin_red | led_pin_green | led_pin_blue;
	gpio_cfg.mode = GPIO_MODE_DEF_OUTPUT;
	gpio_config(&gpio_cfg);

//	gpio_set_direction(led_mb, GPIO_MODE_OUTPUT);
	while(1){
		uroven^= 1;
		gpio_set_level(led_green, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
		gpio_set_level(led_red, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
		gpio_set_level(led_blue, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
		gpio_set_level(led_mb, uroven);
		vTaskDelay(200/portTICK_PERIOD_MS);
	}

}
