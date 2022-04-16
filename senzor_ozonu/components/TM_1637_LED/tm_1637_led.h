/*
 * tm_1637_led.h
 *
 *  Created on: 2. 2. 2022
 *      Author: marcel
 */

#ifndef COMPONENTS_TM_1637_LED_TM_1637_LED_H_
#define COMPONENTS_TM_1637_LED_TM_1637_LED_H_

#include "driver/i2c.h"
#include "driver/gpio.h"

/* pro TM_1637 displej dekuji https://github.com/AlexAlexFr  */
#define TM_1637_DIO	GPIO_NUM_14
#define TM_1637_CLK	GPIO_NUM_16


#define DIO_INP  	gpio_set_direction(TM_1637_DIO, GPIO_MODE_DEF_INPUT)
#define DIO_OUT  	gpio_set_direction(TM_1637_DIO, GPIO_MODE_DEF_OUTPUT)
#define DIO_SET  	gpio_set_level(TM_1637_DIO, 1)
#define DIO_CLR  	gpio_set_level(TM_1637_DIO, 0)
#define DIO_IS_SET  gpio_get_level(TM_1637_DIO)
#define DIO_IS_CLR  !gpio_get_level(TM_1637_DIO)
#define CLK_INP  	gpio_set_direction(TM_1637_CLK, GPIO_MODE_DEF_INPUT)
#define CLK_OUT  	gpio_set_direction(TM_1637_CLK, GPIO_MODE_DEF_OUTPUT)
#define CLK_SET  	gpio_set_level(TM_1637_CLK, 1)
#define CLK_CLR  	gpio_set_level(TM_1637_CLK, 0)
#define TM1637_SERIAL_INIT  CLK_CLR; CLK_INP; DIO_CLR; DIO_INP;

#define TM_1637_GPIO_CONFIG 1

#define CLK_H  CLK_INP
#define CLK_L  CLK_OUT

#define DIO_H  DIO_INP
#define DIO_L  DIO_OUT

#define HIGH  1
#define LOW   0

#define SERIAL_DELAY os_delay_us(10);





#define DATA_COMMAND     0b01000000
#define AUTO_ADDRESS     0b00000000
#define FIXED_ADDRESS    0b00000100
#define WRITE_DATA       0b00000000
#define READ_DATA        0b00000010

#define ADDRESS_COMMAND  0b11000000
#define DEFAULT_ADDRESS  0b00000000

#define DISPLAY_COMMAND  0b10000000
#define DISPLAY_ON       0b00001000
#define DISPLAY_OFF      0b00000000
#define DISPLAY_BRIGHT   0b00000010

enum {
	disp_bright0,
	disp_bright1,
	disp_bright2,
	disp_bright3,
	disp_bright4,
	disp_bright5,
	disp_bright6,
	disp_bright7,
}DISPLAY_BRIGHT_SET;

void led_dots(uint8_t on);
void led_day_set ();
void led_night_set ();
void led_print(uint8_t pos, char *str);
void tm_1637_gpio_init();
void led_dots_test(uint8_t on);


#endif /* COMPONENTS_TM_1637_LED_TM_1637_LED_H_ */
