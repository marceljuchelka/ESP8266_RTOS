/*
 * ads_1115.h
 *
 *  Created on: 9. 11. 2020
 *      Author: marcel
 */

#ifndef ADS_1115_H_
#define ADS_ADS_1115_H_
#include "esp_err.h"

#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */

//adresy ads115
#define ads_i2c_add_0x90		0x90
#define ads_i2c_add_0x91		0x91
#define ads_i2c_add_0x92		0x92
#define ads_i2c_add_0x93		0x93


//Config register   Default = 0x8583h  = 1000 0101 1000 0011
#define ADS_ADS_COMP_QUE0		0
#define ADS_COMP_QUE1 			1
#define ADS_COMP_LAT 			2
#define ADS_COMP_POL 			3
#define ADS_COMP_MODE 			4
#define ADS_DR0 				5
#define ADS_DR1 				6
#define ADS_DR2 				7
#define ADS_MODE				8
#define ADS_PGA0				9
#define ADS_PGA1				10
#define ADS_PGA2				11
#define ADS_MUX_0				12
#define ADS_MUX_1				13
#define ADS_MUX_2				14
#define ADS_OS					15

/*							v/32767
000 : FSR = ±6.144 V		0.1875
001 : FSR = ±4.096 V		0.125
010 : FSR = ±2.048 V (def)	0.0625
011 : FSR = ±1.024 V		0.0312
100 : FSR = ±0.512 V		0.0156
101 : FSR = ±0.256 V		0.007812
110 : FSR = ±0.256 V
111 : FSR = ±0.256 V
 */

//Programmable gain amplifier configuration
#define ADS_FSR0				0	//±6.144 V
#define ADS_FSR1				1	//±4.096 V
#define ADS_FSR2				2	//±2.048 V (default)
#define ADS_FSR3				3	//±1.024 V
#define ADS_FSR4				4	//±0.512 V
#define ADS_FSR5				5	//±0.256 V
#define ADS_FSR6				6	//±0.256 V
#define ADS_FSR7				7	//±0.256 V

//Data rate
#define ADS_DR8					0	//000 : 8 SPS(default)
#define ADS_DR16				1	//001 : 16 SPS
#define ADS_DR32				2	//010 : 32 SPS
#define ADS_DR64				3	//011 : 64 SPS
#define ADS_DR128				4	//100 : 128 SPS
#define ADS_DR250				5	//101 : 250 SPS
#define ADS_DR475				6	//110 : 475 SPS
#define ADS_DR860				7	//111 : 860 SPS


/*mux
000 : AINP = AIN0 and AINN = AIN1 (default)
001 : AINP = AIN0 and AINN = AIN3
010 : AINP = AIN1 and AINN = AIN3
011 : AINP = AIN2 and AINN = AIN3
100 : AINP = AIN0 and AINN = GND
101 : AINP = AIN1 and AINN = GND
110 : AINP = AIN2 and AINN = GND
111 : AINP = AIN3 and AINN = GND
*/

#define ADS_MUX0				0
#define ADS_MUX1				1
#define ADS_MUX2				2
#define ADS_MUX3				3
#define ADS_MUX4				4
#define ADS_MUX5				5
#define ADS_MUX6				6
#define ADS_MUX7				7


/*
Device operating mode
This bit controls the operating mode.
 */
#define ADS_Continuous_mode		0
#define ADS_Single				1

/*pointer registers
00 : Conversion register
01 : Config register
10 : Lo_thresh register
11 : Hi_thresh register
 */
#define ADS_Conversion_register		0
#define ADS_Config_register			1
#define ADS_Lo_thresh_register		2
#define ADS_Hi_thresh_register		3

#define koeficient 				0,032768				//pro vypocet napeti z prevodniku


void ads_init();
void my_i2c_config();
void ads_set_gain(uint8_t gain);
/* nacteni napeti z ads */
esp_err_t ads_read_volt_single(uint8_t MUX,float * Volt);
esp_err_t ads_read_volt_cont(float * Volt);
//void ads_read_config_register();
int8_t ads_test_address(uint8_t adresa);
void ads_write_register(uint8_t APR, uint16_t data);
esp_err_t ads_read_register(uint8_t APR, uint16_t *reg_read);
void ads_set_mux(uint8_t MUX);
void ads_set_datarate(uint8_t DR);
void ads_start_conversion();
void ads_bit_set(uint8_t bit, uint8_t hodnota);
uint8_t ads_bit_test(uint8_t bit);
uint16_t ads_read_single_mux(uint8_t MUX);
uint16_t ads_read_continual_mux(uint8_t MUX);
float ads_U_input_single(uint8_t MUX);
float ads_U_input_cont(uint8_t MUX);


extern const float ads_fsr_table[6];
extern volatile uint8_t ads_OK,ads_i2c_address;
extern volatile uint16_t Buf_Config_register;

#endif /* ADS_1115_H_ */
