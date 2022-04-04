/*
 * mj_am2320b.h
 *
 *  Created on: 1. 10. 2020
 *      Author: marcel
 */

#ifndef MJ_AM2320B_MJ_AM2320B_H_
#define MJ_AM2320B_MJ_AM2320B_H_

#define ACK 1
#define NACK 0
#define AM2320B_address 0xB8
#define I2C_w 0
#define I2C_r 1
#define temperat 1
#define humidy	2
#define AM2320_ERROR 3

float am2320_getdata(uint8_t select);
esp_err_t am2320_test_address();

#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */

enum {
	am2320_KO,
	am2320_OK,
	am2320_error = -123,
};

//typedef union{
//	uint8_t AM_Bytes[6];
//	struct{
//		uint16_t humid;
//		uint16_t temp;
//		uint16_t crc;
//	};
//}AM2320_VAR_T;

typedef union{
	uint8_t AM_Bytes[8];
	struct{
		uint16_t blbost;
		uint8_t humid_hi;
		uint8_t humid_lo;
		uint8_t temp_hi;
		uint8_t temp_lo;
		uint16_t crc;
	};
}AM2320_VAR_T;

#endif /* MJ_AM2320B_MJ_AM2320B_H_ */
