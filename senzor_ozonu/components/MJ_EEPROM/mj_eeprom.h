/*
 * mj_eeprom.h
 *
 *  Created on: 4. 6. 2022
 *      Author: marcel
 */

#ifndef COMPONENTS_MJ_EEPROM_MJ_EEPROM_H_
#define COMPONENTS_MJ_EEPROM_MJ_EEPROM_H_

#include "../MK_WIFI/mk_wifi.h"
#include "../ULP/ulp.h"

typedef struct{
	ULP_DATA_EEPROM ulp_flash;
	T_WIFI_PARAM wifi_flash;
}T_DATA_STORAGE_FLASH;

typedef struct {
	uint32_t	sector_num;
}T_FLASH_SET;


/* parametry pro ukladani dat na flash disk   */
#define flash_start_sector	0x1FC
#define flash_sector_size	0x1000				//velikost sektoru- 4096 Byte
#define flash_data_addres_start		flash_start_sector * flash_sector_size		//adresa ulozeni dat 0x1fc000 = 2 080 768 bytu




enum {
	sector0 ,
	sector1	,
	sector2	,
}T_SECTORS;


esp_err_t write_eeprom(T_DATA_STORAGE_FLASH *flash_data);
esp_err_t read_eeprom(T_DATA_STORAGE_FLASH *flash_data);

#endif /* COMPONENTS_MJ_EEPROM_MJ_EEPROM_H_ */
