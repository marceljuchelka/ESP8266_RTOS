/*
 * PCF8574.c
 *
 *  Created on: 2010-09-07
 *       Autor: Miros³aw Kardaœ
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "../MK_I2C/mk_i2c.h"

#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */




/*
 * 	8-bit EXPANDER
 * 	PDF: https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf
 *
	PCF8574 - Base addr 0x40

	A6|A5|A4|A3|A2|A1|A0|RW|ADR
	------------------------
	 0| 1| 0| 0|  |  |  |  |0x40 <-- base addr

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



	PCF8574A - Base addr 0x70

	A6|A5|A4|A3|A2|A1|A0|RW|ADR
	------------------------
	 0| 1| 1| 1|  |  |  |  |0x70 <-- base addr

	A6|A5|A4|A3|A2|A1|A0|RW|ADR
	------------------------
	 0| 1| 1| 1| 0| 0| 0|  |0x70
	 0| 1| 1| 1| 0| 0| 1|  |0x72
	 0| 1| 1| 1| 0| 1| 0|  |0x74
	 0| 1| 1| 1| 0| 1| 1|  |0x76
	 0| 1| 1| 1| 1| 0| 0|  |0x78
	 0| 1| 1| 1| 1| 0| 1|  |0x7A
	 0| 1| 1| 1| 1| 1| 0|  |0x7C
	 0| 1| 1| 1| 1| 1| 1|  |0x7E
*/


#if I2C_MODE == 0

	/*konfigurace i2c na ESP  vcetne driveru*/
void my_i2c_pcf8574_config(){
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

void pcf8574_write( uint8_t SLA, uint8_t byte ) {
//	esp_err_t ret;
	i2c_cmd_handle_t cmd;
//	my_i2c_pcf8574_config();
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SLA, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, byte, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
	I2C_TAKE_MUTEX_NORET;
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	I2C_GIVE_MUTEX_NORET;
	i2c_cmd_link_delete(cmd);
//	i2c_driver_delete(I2C_NUM_0);

//	i2c_start();
//	i2c_write(SLA);
//	i2c_write( byte );
//	i2c_stop();
//	PORTC ^= (1<<PC6);
}

uint8_t pcf8574_read( uint8_t SLA ) {
	uint8_t res = 0;
//	esp_err_t ret = 0;
	i2c_cmd_handle_t cmd;
//	my_i2c_pcf8574_config();
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SLA+1, I2C_MASTER_ACK);
	i2c_master_read_byte(cmd, &res, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	I2C_TAKE_MUTEX;
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);
//	i2c_driver_delete(I2C_NUM_0);


//	i2c_start();
//	i2c_write(SLA+1);
//	res = i2c_read( NACK );
//	i2c_stop();

	return res;
}

#endif
