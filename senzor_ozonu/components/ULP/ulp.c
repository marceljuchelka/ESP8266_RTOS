/*
 * ulp.c
 *
 *  Created on: 17. 5. 2020
 *      Author: Marcel Juchelka
 *      senzor ULP pro O3 a mereni pomoci ads1115 I2C AD prevodniku
 *      Nacitani co 200ms ukladani do fronty a 1x za sekundu se ulozi do fronty ozonHandle kde je neustale prepisovana
  */



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "queue.h"
#include "event_groups.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "math.h"
#include "../ADS_1115/ads_1115.h"
#include "../ULP/ulp.h"
#include "../MJ_EEPROM/mj_eeprom.h"
//#include "ADS_1115/ads_1115.h"

ULP_VAR_STRUCT _ULP_promenne_global = {.M_span = M_span_def,.sens_code = Sens_code_def,.PPM_select = PPM_sel_def};
ULP_pins_U ULP_pins_U_global;

QueueHandle_t fronta_vzorku_napeti;				//fronta na nacitani hodnot z prevodniku
QueueHandle_t	OzonHandle;						//aktualni hodnota ozonu
EventGroupHandle_t	xEventUlpHandle;		//info o stavu ozonoveho senzoru


volatile uint8_t ulp_OK = 0, ULP_MUX_FLAG = 0;

/* ******************************************************RTOS  *********************************/
TaskHandle_t	PPMReadHandle;
TaskHandle_t 	VoltagereadHandle;



void ULP_set_cont(void *arg){
	ads_set_mux(ulp_Vgas_read);
	ads_bit_set((ADS_MODE),ADS_Continuous_mode);						//single or Continuous-conversion mode
}

/* nacitani hodnot ze senzoru  	xTaskCreate(vULP_VoltageRead, "voltage read", 1300, NULL, 1, voltagereadHandle); */
void vULP_VoltageRead(void *arg) {
	uint8_t delay = 0;
	const char *TAG = "ULP_voltage read";
	OzonHandle = xQueueCreate(1, sizeof(float));												//aktualni hodnota ozonu
	fronta_vzorku_napeti = xQueueCreate(5, sizeof(float));										//fronta nacitani hodnot y cidla
	ULP_set_cont(0);
	ads_set_mux(ulp_Vgas_read);
	float volt = 0;
	while (1) {
		if ((xEventGroupGetBits(xEventUlpHandle) & uxUlmCalibrate)) {							//neni li kalibrace
			if (ads_read_volt_cont(&volt) == ESP_OK) {//nacteni hodnoty s pinu Vgas
				if (xQueueSendToBack(fronta_vzorku_napeti,&volt,0) != pdTRUE) {
					vTaskResume(PPMReadHandle);
					delay = 20;
				}
			}
			else xEventGroupClearBits(xEventUlpHandle,uxUlmPresent | uxUlmInit | uxUlmCalibrate | uxUlmPrevedeno);
		} else {
			xEventGroupClearBits(xEventUlpHandle,uxUlmPresent | uxUlmInit | uxUlmCalibrate | uxUlmPrevedeno);
			xQueueReset(fronta_vzorku_napeti);
			ESP_LOGE(TAG, "*** ULP_odpojeno ***");
			delay = 100;
			ULP_seting();
		}
		vTaskDelay(delay);
	}
}

/* nacteni a vypocet PPM  ulozeni do ozon queue xTaskCreate(vULP_PPM_read, "PPM read", 1300, NULL, 1, &PPMReadHandle)*/
void vULP_PPM_read(void *arg) {
//	const char* TAG =  "PPM_read";
	float PPM, DataZFronty, prumer = 0;
	while (1) {
		while (xQueueReceive(fronta_vzorku_napeti, &DataZFronty, 10)) {
			prumer = prumer + DataZFronty;
		};
		ULP_pins_U_global.Vgas_U = prumer/5;
//		printf("V_gas > %f     V_ref> %f    V_batt> %f  Voffset> %f\n", ULP_pins_U_global.Vgas_U, ULP_pins_U_global.Vref_U, ULP_pins_U_global.Vbatt_U, ULP_pins_U_global.Voffset_U);
		PPM = (ULP_pins_U_global.Vref_U - ULP_pins_U_global.Voffset_U - ULP_pins_U_global.Vgas_U) / _ULP_promenne_global.M_span;
		xQueueOverwrite(OzonHandle,&PPM);
		prumer = 0;
		vTaskSuspend(NULL);
	}
}

esp_err_t vULP_kalibrace() {
	const char *TAG = "ULP kalibrace";
	float prumer = 0, napeti = 0;
	T_DATA_STORAGE_FLASH ulozena_data;
	uint8_t opakovani = 5;										//, pokusy = 10;

	ads_bit_set((ADS_MODE), ADS_Continuous_mode);//single or Continuous-conversion mode

	ads_set_mux(ulp_Vbat_read);				//zjisteni napeti baterie
	vTaskDelay(20);
	ads_read_volt_cont(&napeti) ;
	ULP_pins_U_global.Vbatt_U = napeti;

	ads_set_mux(ulp_Vref_read);				//zjisteni napeti referencniho
	vTaskDelay(20);
	ads_read_volt_cont(&napeti) ;
	ULP_pins_U_global.Vref_U = napeti;

	ads_set_mux(ulp_Vgas_read);				//nacteni vzorku pro prumer vgas
	while (opakovani--) {
		vTaskDelay(20);
		ads_read_volt_cont(&napeti) ;
		prumer = prumer + napeti;
	}
	ULP_pins_U_global.Vgas_U = prumer / 5;

	ULP_pins_U_global.Voffset_U = ULP_pins_U_global.Vref_U 	- ULP_pins_U_global.Vgas_U;
	ESP_LOGI(TAG, "V_gas > %f     V_ref> %f    V_batt> %f  Voffset> %f\n", 	ULP_pins_U_global.Vgas_U, ULP_pins_U_global.Vref_U,	ULP_pins_U_global.Vbatt_U, ULP_pins_U_global.Voffset_U);
	if (ULP_pins_U_global.Voffset_U > 0.0) {
		ESP_LOGI(TAG, "*** kalibrovano ***");
		xEventGroupSetBits(xEventUlpHandle, uxUlmCalibrate);
		read_eeprom(&ulozena_data);
		ulozena_data.ulp_flash.ulp_pins_eeprom.Voffset_U = ULP_pins_U_global.Voffset_U;
		write_eeprom(&ulozena_data);
		return ESP_OK;
	}
	return ESP_FAIL;
}


esp_err_t ULP_seting(){
	const char* TAG =  "ULP_setting";
	uint16_t config_register;
	float napeti;
	ads_i2c_address = ULP_ADS_address;
	if(ads_test_address(ads_i2c_address) == ESP_OK) {									//testovani je li prevodnik
		xEventGroupSetBits(xEventUlpHandle, uxUlmPresent);								//odpovida li modul
		ESP_LOGI(TAG, "*** present ***");
		ulp_OK = 1;																		//prevodnik je na adrese
		Buf_Config_register = 0x8583;
		ads_write_register(ADS_Config_register,Buf_Config_register);						//reset config
		ads_read_register(ADS_Config_register,&config_register);		//nacte config register do Buf_Config_register
		ads_set_datarate(ADS_DR8);											//100 : 8 SPS
		ads_set_gain(ADS_FSR1);												//001 : FSR = ±4.096 V
		ads_bit_set((ADS_MODE),ADS_Single);									//single or Continuous-conversion mode
		ads_read_register(ADS_Config_register,&config_register);
		ads_read_volt_single(ulp_Vbat_read, &napeti);
		printf("ADS register2 > 0X %x\n", Buf_Config_register);
		xEventGroupSetBits(xEventUlpHandle, uxUlmInit);						//probehla inicializace
		ESP_LOGI(TAG,"*** inicializace ***");
		if(!(xEventGroupGetBits(xEventUlpHandle) &uxUlmCalibrate)) vULP_kalibrace();	//neni li zkalibrovano zkalibruj
	}
	else{
		xEventGroupClearBits(xEventUlpHandle, uxUlmPresent|uxUlmInit|uxUlmCalibrate|uxUlmPrevedeno);
		ESP_LOGE(TAG,"modul ULC chybi na adrese %x", ULP_ADS_address);
		return ESP_FAIL;
	}
	return ESP_OK;
}

/*inicializace ULP I2C */
esp_err_t ULP_init(){							//nastaveni prevodniku s O3 senzorem
	const char* TAG =  "ULP_init";
	xEventUlpHandle = xEventGroupCreate();
	ESP_LOGI(TAG,"start init %X ", ULP_ADS_address);
	return ULP_seting();
}


/* vypocet naklonu krivky */
#define pocetvpoli 10
float ULP_linreg(float ppm) {
	static float pole_PPM[pocetvpoli];
	uint8_t n = pocetvpoli;
		for(uint8_t ii = 0; ii<pocetvpoli-1; ii++){
			pole_PPM[ii] = pole_PPM[ii+1];
		}
		pole_PPM[pocetvpoli-1] = ppm;

    float   sumx = 0.0;                      /* sum of x     */
    float   sumx2 = 0.0;                     /* sum of x**2  */
    float   sumxy = 0.0;                     /* sum of x * y */
    float   sumy = 0.0;                      /* sum of y     */

    for (int i=0;i<n;i++){
		sumx  += i;
		sumx2 += i*i;
		sumxy += i * pole_PPM[i];
		sumy  += pole_PPM[i];
    }

    float denom = (n * sumx2 - sumx*sumx);

    if (denom == 0) {
//		printf("Sing.");

    }

    return (n * sumxy  -  sumx * sumy) / denom;
}

