/*
 * mj_am2320b.c
 *
 *  Created on: 1. 10. 2020
 *      Author: marcel
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "mj_am2320b.h"
#include "sdkconfig.h"




	/*konfigurace i2c na ESP  vcetne driveru*/
void my_i2c_am2320_config(){
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

uint16_t _calc_crc16(const uint8_t *buf, size_t len) {
  uint16_t crc = 0xFFFF;

  while(len--) {
    crc ^= (uint16_t) *buf++;
    for (unsigned i = 0; i < 8; i++) {
      if (crc & 0x0001) {
	crc >>= 1;
	crc ^= 0xA001;
      } else {
	crc >>= 1;
      }
    }
  }

  return crc;
}

float am2320_getdata(uint8_t select)
{
	i2c_cmd_handle_t cmd;
	uint16_t humid = 0, crc = 0, ret = 0;
	int16_t temp = 0;
	AM2320_VAR_T AM_val;
	uint8_t i = 0;
	int teplota;

//	my_i2c_am2320_config();

	/*probuzeni sensoru 800us az 3ms  */
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AM2320B_address) |I2C_MASTER_WRITE, I2C_MASTER_NACK);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);			//odesli
	i2c_cmd_link_delete(cmd);
	vTaskDelay(1/portTICK_PERIOD_MS);
	cmd = i2c_cmd_link_create();
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS)	;	//odesli
	i2c_cmd_link_delete(cmd);
	vTaskDelay(1/portTICK_PERIOD_MS);
	/* nastaveni commandu nacti 4 byte */
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AM2320B_address) |I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, 0x03, I2C_MASTER_ACK);									//prikaz 3
	i2c_master_write_byte(cmd, 0x00, I2C_MASTER_ACK);									//od registru 0
	i2c_master_write_byte(cmd, 0x04, I2C_MASTER_ACK);									//4 byte
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);					//proved
	i2c_cmd_link_delete(cmd);

	vTaskDelay(10/portTICK_PERIOD_MS);

	/*nacti pozadovane byte ze zarizeni prvni dva byte jsou blbosti */
	cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AM2320B_address) | I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	os_delay_us(50);
	cmd = i2c_cmd_link_create();
	i2c_master_read(cmd, &AM_val.AM_Bytes[0], 8, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
//	i2c_driver_delete(I2C_NUM_0);
	/*kontrola crc - velmi dekuji Joakim Lotsengård*/
	crc = AM_val.crc;
	uint16_t crc_data = _calc_crc16(&AM_val.AM_Bytes[0], 6);
	if(crc_data != crc){
		return -99;
	}

	humid = AM_val.humid_hi;
	humid = (humid<<8) + AM_val.humid_lo;
//	printf("humid =%d\n",humid);
	temp = AM_val.temp_hi;
	temp = (temp<<8) + AM_val.temp_lo;
	if (temp & 0x8000) teplota = 0 - (temp & 0x7fff);	// zaporna teplota?
	else {
		teplota = temp;							//kladna teplota
	}
	if (select == temperat) {
		return (float)teplota/10;			//navrat teploty / 10
	}
	else {
		return (float)humid/10;								//navrat vlhkosti /10
	}
	return AM2320_ERROR;								//jinak navrat error
}

esp_err_t am2320_test_address(){
	esp_err_t chyba = 0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AM2320B_address) |I2C_MASTER_WRITE, I2C_MASTER_ACK);
	chyba = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return chyba;
}

unsigned short crc16(unsigned char *ptr, unsigned char len){
	 unsigned short crc =0xFFFF;
	 unsigned char i;
	 while(len--){
		 crc ^=*ptr++;
		 for(i=0;i<8;i++){
			 if(crc & 0x01){
				 crc>>=1;
				 crc^=0xA001;
			 }
			 else{
				 crc>>=1;
			 }
		 }
	 }
 return crc;
}


