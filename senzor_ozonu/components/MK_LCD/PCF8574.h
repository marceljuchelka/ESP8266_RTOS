/*
 * PCF8574.h
 *
 *  Created on: 1. 1. 2022
 *      Author: marcel
 */

#ifndef COMPONENTS_MK_LCD_PCF8574_H_
#define COMPONENTS_MK_LCD_PCF8574_H_
void my_i2c_pcf8574_config();
void pcf8574_write( uint8_t SLA, uint8_t byte );
uint8_t pcf8574_read( uint8_t SLA );

#endif /* COMPONENTS_MK_LCD_PCF8574_H_ */
