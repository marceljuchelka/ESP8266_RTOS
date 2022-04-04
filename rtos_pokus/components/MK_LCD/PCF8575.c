/*
 * PCF8574.c
 *
 *  Created on: 2010-09-07
 *       Autor: Miros³aw Kardaœ
 */
#include <stdio.h>
#include <driver/i2c.h>


#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */
/*
 * 16-bit EXPANDER
 * PDF: https://www.nxp.com/docs/en/data-sheet/PCF8575.pdf?
 *
	PCF8575 - Base addr 0x40

	A6|A5|A4|A3|A2|A1|A0|RW|ADR
	------------------------
	 0| 1| 0| 0| x| x| x|  |0x40 <-- base addr



	A6|A5|A4|A3|A2|A1|A0|RW|ADR
	------------------------
	 0| 1| 0| 0| 0| 0| 0|  |0x40
	 0| 1| 0| 0| 0| 0| 1|  |0x42
	 0| 1| 0| 0| 0| 1| 0|  |0x44
	 0| 1| 0| 0| 0| 1| 1|  |0x46
	 0| 1| 0| 0| 1| 0| 0|  |0x48
	 0| 1| 0| 0| 1| 0| 1|  |0x4A
	 0| 1| 0| 0| 1| 1| 0|  |0x4C
	 0| 1| 0| 0| 1| 1| 1|  |0x4E

*/


#if I2C_MODE == 0

	/*konfigurace i2c na ESP  vcetne driveru*/
void my_i2c_pcf8575_config(){
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

void pcf8575_write( uint8_t SLA, uint16_t data ) {
	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	my_i2c_pcf8575_config();
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SLA, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, ( data >> 8 ), I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, ( data >> 0 ), I2C_MASTER_ACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	i2c_driver_delete(I2C_NUM_0);

//	i2c_start();
//	i2c_write(SLA);
//	i2c_write( data >> 8 );
//	i2c_write( data >> 0 );
//	i2c_stop();
}

uint16_t pcf8575_read( uint8_t SLA ) {

	uint16_t res = 0;
	uint8_t * buf = (uint8_t*)&res;
	uint8_t len = 2;

	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	my_i2c_pcf8575_config();
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SLA+1, I2C_MASTER_ACK);
	i2c_master_read_byte(cmd, buf, I2C_MASTER_ACK);
	i2c_master_read_byte(cmd, buf+1, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	i2c_driver_delete(I2C_NUM_0);

//	i2c_start();
//	i2c_write(SLA+1);
//	while (len--) *buf++ = i2c_read( len ? ACK : NACK );
//	i2c_stop();

	return res;
}

#endif
