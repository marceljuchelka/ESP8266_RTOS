/*                  e-gadget.header
 * hdc1080.c
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

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "queue.h"
#include "task.h"
#include "driver/i2c.h"
#include "hdc1080.h"
#include "../MK_I2C/mk_i2c.h"
#include "sdkconfig.h"


//	/*konfigurace i2c na ESP  vcetne driveru*/
//void my_i2c_config(){
//	i2c_config_t conf;
//	conf.mode = I2C_MODE_MASTER;
//	conf.sda_io_num = I2C_SDA_PIN;
//    conf.sda_pullup_en = 1;
//	conf.scl_io_num = I2C_SCL_PIN;
//    conf.scl_pullup_en = 1;
//	conf.clk_stretch_tick = 100;
//	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
//	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
//}

QueueHandle_t	TempHandle;			//fronta s teplotou
QueueHandle_t	Humhandle;			//fronta s vlhkosti
TaskHandle_t	hdc1080Task;		//task pro periodicke nacitani vlhkosti a teploty

uint16_t swap_uint16(uint16_t swap_num){
	uint8_t byte_A;
	SWAP16 byte_swap;
	byte_swap.BYTES2 = swap_num;
	byte_A = byte_swap.MSB;
	byte_swap.MSB = byte_swap.LSB;
	byte_swap.LSB = byte_A;
	return byte_swap.BYTES2;
}

esp_err_t hdc_start_mereni(reg_map reg_){
	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, reg_, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX;
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
//	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK) printf("chyba start mereni\n");
	return ret;
}

/*zapis hodnot do registru programovani HDC1080*/
void hdc1080_write_register(reg_map reg_, uint16_t reg_value){
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, reg_, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, (reg_value>>8), I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, (reg_value), I2C_MASTER_ACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX_NORET;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba reg write");
//	I2C_GIVE_MUTEX_NORET;
	i2c_cmd_link_delete(cmd);
}

/* nastaveni registru pred ctenim */
void hdc1080_set_register(reg_map reg_set){
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, reg_set, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX_NORET;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba reg set");
//	I2C_GIVE_MUTEX_NORET;
	i2c_cmd_link_delete(cmd);
}


uint16_t hdc1080_read_register(reg_map set_register){
	uint16_t res = 0;
	uint8_t * buf = (uint8_t*)&res;
	hdc1080_set_register(set_register);
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1)| I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read(cmd, buf, 2, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba reg read");
//	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);
	return swap_uint16(res);

}

float hdc1080_read_hum(){
	/*nastaveni registru */
	dev_config config_register = 0;
	HDC1080_READ_VALUE hodnoty;
	config_register = (HumMR_11_bit << HRES) | (TempMR_11_bit << TRES) | (mode_0 << MODE);
	hdc1080_write_register(reg_Configuration, config_register);

	/*start mereni od */
	hdc_start_mereni(reg_Humidity);

	/* cekani na prevod */
	vTaskDelay(12/portTICK_PERIOD_MS);

	/* stazeni 2 bytu */
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1)| I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read(cmd, &hodnoty.bytes[2], 2, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba hum");
//	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);

	/* vypocet hodnot  */
	float hum_F = swap_uint16(hodnoty.humid_r);
	hum_F  = ((float)hum_F*100)/65536;
 	return hum_F;
}

float hdc1080_read_temp(){
	uint16_t temp = 0;
	HDC1080_READ_VALUE hodnoty;
	float temp_F = 0;
	/*nastaveni registru */
	dev_config config_register = 0;
	config_register = (HumMR_11_bit << HRES) | (TempMR_11_bit << TRES) | (mode_0 << MODE);
	hdc1080_write_register(reg_Configuration, config_register);

	/*start mereni  */
	hdc_start_mereni(reg_Temperature);

	/*cekani na prevod*/
	vTaskDelay(12/portTICK_PERIOD_MS);
	/*nacteni 2 byte */
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1)| I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read(cmd, &hodnoty.bytes[0], 2, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba temp");
//	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);

	/* vypocet hodnoty */
	temp = swap_uint16(hodnoty.temp_r);
	temp_F = ((((float)temp)/(float)0xFFFF)*((float)165)-(float)40);
	return temp_F;
}
esp_err_t hdc1080_measure(float *temp, float *hum){
	HDC1080_READ_VALUE hodnoty;
	dev_config config_register = 0;
	config_register = (HumMR_11_bit << HRES) | (TempMR_11_bit << TRES) | (mode_1 << MODE);
	hdc1080_write_register(reg_Configuration, config_register);
	hdc_start_mereni(reg_Temperature);
	vTaskDelay(20/portTICK_PERIOD_MS);
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1)| I2C_MASTER_READ, I2C_MASTER_ACK);
	i2c_master_read(cmd, &hodnoty.bytes[0], 4, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
//	I2C_TAKE_MUTEX;
	if(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS) != ESP_OK) printf("chyba measure");
//	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);
//	printf("readtemppraw = %X\n", hodnoty.temp_r);
//	printf("readhumpraw = %X\n", hodnoty.humid_r);
	*hum = ((float)swap_uint16(hodnoty.humid_r)*100)/65536;
	*temp = swap_uint16(hodnoty.temp_r);
	*temp = (*temp)/(0xFFFF)*(165)-(40);
//	printf("temp = %2.2f",*temp);
 	return ESP_OK;
}


esp_err_t hdc1080_test(){
	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (hdc_1080_address <<1)|I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_stop(cmd);
	I2C_TAKE_MUTEX;
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	I2C_GIVE_MUTEX;
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK){
		printf("neni spojeni s hdc1080\n");
		return ESP_FAIL;
	}
	return ESP_OK;
}


void hdc1080_init(){
	dev_config config_register = 0;
	printf("config register init = %X\n", hdc1080_read_register(reg_Configuration));
	config_register|= (HumMR_11_bit << HRES) | (TempMR_11_bit << TRES) | (mode_1 << MODE);
	hdc1080_write_register(reg_Configuration, config_register);
	printf("config register init po zapisu = %X\n", hdc1080_read_register(reg_Configuration));
	printf("config registrer device ID = %d\n", hdc1080_read_register(reg_DeviceID));
}

/* task na pravidelne cteni teploty a vlhkosti  - ulozeni do front */
void hdc1080_read (void *arg){
	static char *TAG = "hdc1080_Read";
	float temp, hum;
	uint16_t delay;
	TempHandle = xQueueCreate(1,sizeof(float));
	Humhandle = xQueueCreate(1,sizeof(float));
	while(1){
		if(i2c_take_mutex() == ESP_OK){
//			ESP_LOGI(TAG,"OK nacteni I2C");
//			temp = hdc1080_read_temp();
//			hum = hdc1080_read_hum();
			hdc1080_measure(&temp, &hum);
			xQueueSendToBack(TempHandle,&temp,0);
			xQueueSendToBack(Humhandle,&hum,0);
//			printf("--------------------temp = %.1f  Hum = %.1f\n",temp,hum);
			delay = 500;
			I2C_GIVE_MUTEX_NORET;
		}
		else {
			ESP_LOGE(TAG,"chyba nacteni I2C");
			delay = 5;
		}
		vTaskDelay(delay);
	}
}
