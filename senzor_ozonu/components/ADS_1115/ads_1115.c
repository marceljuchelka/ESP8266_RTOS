/*
 * ads_1115.c
 *
 *  Created on: 9. 11. 2020
 *      Author: marcel
 *
 *      Data Rate Setting   DRS
 *      pred pouzitim init
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "../ADS_1115/ads_1115.h"
#include "../ULP/ulp.h"


volatile uint16_t Buf_Config_register;
volatile uint8_t ads_OK,ads_i2c_address = ads_i2c_add_0x90;
const float ads_fsr_table[6] = {0.1875,0.125,0.0625,0.03125,0.01562,0.007812}; 		//v uV/10		//pro vypocet napeti z prevodniku


void my_i2c_config(){
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_SDA_PIN;
    conf.sda_pullup_en = 1;
	conf.scl_io_num = I2C_SCL_PIN;
    conf.scl_pullup_en = 1;
	conf.clk_stretch_tick = 100;
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
}

uint16_t ads_read_register(uint8_t APR){								//precti register
	uint8_t buffer[3];
	uint8_t *buf = &buffer[0];

	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ads_i2c_address | I2C_MASTER_WRITE, I2C_MASTER_NACK);
	i2c_master_write_byte(cmd, APR, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ads_i2c_address | I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read(cmd, buf, 2, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK) printf("chyba read register %d \n" , ESP_OK);
	return ((buffer[0]<<8) + buffer[1]);

//	i2c_send_byte(ads_i2c_address,APR);
//	i2c_read_buf1(ads_i2c_address,2,buf);
//	return ((buffer[0]<<8) + buffer[1]);

}

/*
zapis do registru - APR Address Pointer Register urcuje do ktereho se bude zapisovat
*/
void ads_write_register(uint8_t APR, uint16_t data){
	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ads_i2c_address| I2C_MASTER_WRITE, I2C_MASTER_NACK);
	i2c_master_write_byte(cmd, APR, I2C_MASTER_NACK);
	i2c_master_write_byte(cmd, ( data >> 8 ), I2C_MASTER_NACK);
	i2c_master_write_byte(cmd, ( data >> 0 ), I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK) printf("chyba write register  ESP_: %d \n ", ret);

//	i2c_start();
//	i2c_write(ads_i2c_address);
//	i2c_write(APR);
//	i2c_write( data >> 8 );
//	i2c_write( data >> 0 );
//	i2c_stop();

}
void ads_init(){
	if(ads_test_address(ads_i2c_address) == ESP_OK) {									//testovani je li prevodnik
		printf("bufconfigregister 1: %x\n", ads_read_register(ADS_Config_register));
		ads_OK = 1;															//prevodnik je na adrese
		ads_write_register(ADS_Config_register,0x8583);						//reset config
		ads_write_register(ADS_Config_register,0x1234);						//reset config
		Buf_Config_register = ads_read_register(ADS_Config_register);		//nacte config register do Buf_Config_register
//		lcd_bin_al(0,0,Buf_Config_register,16,_left);
//		ads_set_mux(ADS_MUX4);												//nastavi 100 : AINP = AIN0 and AINN = GND
		printf("bufconfigregister 2: %x\n", Buf_Config_register);
		ads_set_datarate(ADS_DR8);											////000 : 8 SPS(default)
		ads_set_gain(ADS_FSR1);												//001 : FSR = ±4.096 V
		ads_bit_set((ADS_MODE),ADS_Single);									//Continuous-conversion mode
		Buf_Config_register = ads_read_register(ADS_Config_register);
	}
}

void ads_set_gain(uint8_t FSR){
	Buf_Config_register&=0xf1ff;										//vynuluj gain
	Buf_Config_register|= ((uint16_t)FSR << 9);
	ads_write_register(ADS_Config_register,Buf_Config_register);
}
void ads_set_mux(uint8_t MUX){
	Buf_Config_register&=0x8fff;										//vynuluj MUX
	Buf_Config_register|= ((uint16_t)MUX << 12);
	ads_write_register(ADS_Config_register,Buf_Config_register);
}
void ads_set_datarate(uint8_t DR){
	Buf_Config_register&=0xFF1F;										//vynuluj datarate
	Buf_Config_register|= ((uint16_t)DR << 5);
	ads_write_register(ADS_Config_register,Buf_Config_register);
}
int8_t ads_test_address(uint8_t adresa){

	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ads_i2c_address | I2C_MASTER_WRITE, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
//	printf("ret = %d  \n", ret);
	i2c_cmd_link_delete(cmd);
	return ret;





//	i2c_start();
//	if(i2c_write(adresa)) {
//		i2c_stop();
//		return 1;
//	}
//	return 0;
}

void ads_start_conversion(){
	Buf_Config_register|= (1<<ADS_OS);									//nastav 1 na OS
	ads_write_register(ADS_Config_register,Buf_Config_register);
}
//void ads_read_config_register(){
//	uint8_t buffer[3];
//	uint8_t *buf = &buffer[0];
//	i2c_send_byte(ads_i2c_address,ADS_Config_register);
//	i2c_read_buf1(ads_i2c_address,2,buf);
//	Buf_Config_register=((buffer[0]<<8) + buffer[1]);
//	lcd_hex_al(1,0,Buf_Config_register,_left);
//	_delay_ms(5000);
//}




void ads_bit_set(uint8_t bit, uint8_t hodnota){											//nastaveni bitu v registru
	if(hodnota)	Buf_Config_register|=(1<<bit);
	else Buf_Config_register&= ~(1<<bit);
	ads_write_register(ADS_Config_register,Buf_Config_register);
}

uint8_t ads_bit_test(uint8_t bit){
	if(ads_read_register(ADS_Config_register) & (1<<bit)) return 1;
	else return 0;
}

uint16_t ads_read_single_mux(uint8_t MUX){						//nacteni hodnoty v single modu z urciteho MUX

	ads_set_mux(MUX);
	ads_start_conversion();
	vTaskDelay(13);
	while(!(ads_bit_test(ADS_OS))) ;							//cekani na ukonceni prevodu
	return ads_read_register(ADS_Conversion_register);

}

uint16_t ads_read_continual_mux(uint8_t MUX){
	return ads_read_register(ADS_Conversion_register);
}

float ads_U_input_single(uint8_t MUX){
	uint8_t result_PGA;
//	uint16_t hodnota;
	result_PGA= (Buf_Config_register>>ADS_PGA0) & 3;
	return ads_read_single_mux(MUX) * ads_fsr_table[result_PGA] ;								//*fsr tabulka
}

float ads_U_input_cont(uint8_t MUX){
	uint8_t result_PGA;
	float vypocet;
	ads_set_mux(MUX);
	result_PGA= (Buf_Config_register>>ADS_PGA0) & 3;
	vypocet = ads_read_continual_mux(MUX) * ads_fsr_table[result_PGA] ;								//*pgm_read_float(ads_fsr_table[1]
	return vypocet;
}
