/*
 * mj_eeprom.c
 *
 *  Created on: 4. 6. 2022
 *      Author: marcel
 */



#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_spi_flash.h"
#include "mj_eeprom.h"
#include "sdkconfig.h"

T_DATA_STORAGE_FLASH data_storage;
