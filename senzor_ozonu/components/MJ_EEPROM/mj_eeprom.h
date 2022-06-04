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



#endif /* COMPONENTS_MJ_EEPROM_MJ_EEPROM_H_ */
