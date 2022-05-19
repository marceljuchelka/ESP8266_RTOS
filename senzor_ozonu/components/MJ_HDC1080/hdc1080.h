/*                  e-gadget.header
 * hdc1080.h
 *
 *  Created on: 10.06.2021
 *    Modyfied: 10.06.2021 15:38:32
 *      Author: Marcel Juchelka
 *
 * Project name: "test_HDC1080_I2C"
 *
 *
 *          MCU: ATmega328P
 *        F_CPU: 8 000 000 Hz
 *
 *    Flash: 3 588 bytes   [ 10,9 % ]
 *      RAM:  11 bytes   [ 0,5 % ]
 *   EEPROM:  0 bytes   [ 0,0 % ]
 *
 */

#ifndef MJ_HDC1080_HDC1080_H_
#define MJ_HDC1080_HDC1080_H_

#include "queue.h"

void my_i2c_config();


#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */
//#define I2C_1_MASTER_NUM            	I2C_NUM_0        /*!< I2C port number for master dev */
#define test

#define hdc_1080_address 0x40



typedef struct{
	float HDC1080_teplota;
	float HDC1080_vlhkost;
}HDC_VAR_T;

typedef union{
	uint16_t BYTES2;
	struct{
		uint8_t LSB;
		uint8_t	MSB;
	};
}SWAP16;

typedef union{
	uint8_t bytes[4];
	struct{
		uint16_t temp_r;
		uint16_t humid_r;
	};
}HDC1080_READ_VALUE;

enum{
	HDC_temperature,
	HDC_humidy,
};

enum {
	_i2c_write,
	_i2c_read,

};

/* mapa registru obsahuje datové záznamy, které obsahují informace o konfiguraci,
 * výsledky mìøení teploty a vlhkosti a stavové informace. */
typedef enum {
	reg_Temperature 	= 	0x00,
	reg_Humidity 		= 	0x01,
	reg_Configuration 	=	0x02,
	reg_SerialID1 		= 	0xFB,
	reg_SerialID2 		= 	0xFC,
	reg_SerialID3 		=	0xFD,
	reg_ManufacturerID	=	0xFE,
	reg_DeviceID 		=	0xFF,
}reg_map;

/* nasaveni konfiguracniho registru v bitech
 * Tento registr konfiguruje funkènost zaøízení a vrací stav. */
typedef enum {
	HRES = 	8,				//0x0100,
	TRES = 	10,				//0x0400,
	BTST = 	11,				//0x0800,
	MODE = 	12,				//0x1000,
	HEAT =	13,				//0x2000,
	RST = 	15,				//0x8000,
}dev_config;

typedef enum{
	mode_0 = 0,
	mode_1 = 1,
}dev_mode;

typedef enum {
	TempMR_11_bit = 0,
	TempMR_14_bit = 1,
}temp_res;

typedef enum {
	HumMR_14_bit = 0b00,
	HumMR_11_bit = 0b01,
	HumMR_8_bit = 0b10,
}hum_res;

extern QueueHandle_t	TempHandle;			//fronta s teplotou
extern QueueHandle_t	Humhandle;			//fronta s vlhkosti
extern TaskHandle_t		hdc1080Task;		//task pro periodicke nacitani vlhkosti a teploty

uint16_t swap_uint16(uint16_t swap_num);
void hdc1080_init();
uint16_t hdc1080_read_register(reg_map set_register);
float hdc1080_read_hum();
float hdc1080_read_temp();
esp_err_t hdc1080_measure(float *temp, float *hum);
int hdc1080_test();
void hdc1080_read (void *arg);

#endif /* MJ_HDC1080_HDC1080_H_ */

