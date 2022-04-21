/*
 * ulp.h
 *
 *  Created on: 17. 5. 2020
 *      Author: Marcel Juchelka
 */

#ifndef ULP_ULP_H_
#define ULP_ULP_H_

#include "../ADS_1115/ads_1115.h"
#include "queue.h"

#define ULP_ADS_address 		ads_i2c_add_0x90				//adresa prevodniku pro ULP
//vystupy na senzoru
#define Vgas_pin 		1			//vystup Vgas na senzoru ULP
#define Vref_pin 		2			//vystup Vref na senzoru ULP
#define Vtemp_pin		3			//vystup Vtemp na senzoru ULP
#define Vbattery_pin	4			//vystup Vbaterie na senzoru ULP

//vstupy na AD prevodniku
#define ulp_Vgas_read  	ADS_MUX4	//hodnota z Vgas
#define ulp_Vref_read	ADS_MUX5	//hodnota z Vref
#define ulp_Vtemp_read	ADS_MUX6	//hodnota z Vtemp
#define ulp_Vbat_read	ADS_MUX7	//hodnota z Vbattery

//zakladni preddefinovane hodnoty
#define 	U_Bat_min	2800		//minimalni napeti baterie 2800mV (Vref * 2)
#define 	_TIAgain	499
#define 	PPM_sel_def		3
#define 	Sens_code_def	45610		//prace 53920			//hodnota sensivity code ze senzoru
#define 	M_span_def		(((float)Sens_code_def/1000)*((float)_TIAgain))/1000		//defaultni napeti		mV/PPM

//pozadavky na system
#define 	PPM_minim	10



enum {ulp_error = -1,ulp_disable,ulp_enable};

typedef struct{
	float Vgas_U;
	float Vref_U;
	float Vtemp_U;
	float Voffset_U;
	float Vbatt_U;
} ULP_pins_U;					//napeti hna pinech ULP

typedef struct{
	float 		M_span;				//vypoctene mV/PPM
	uint32_t	sens_code;			//citlivost z QR code ze sensoru
	uint8_t		PPM_select;			//nastaveni pozadovaneho PPM
} ULP_VAR_STRUCT;

uint8_t		PPM_select;


extern QueueHandle_t	OzonHandle;
extern TaskHandle_t		PPMReadHandle;
extern TaskHandle_t 	VoltagereadHandle;
extern QueueHandle_t 	fronta_vzorku_napeti;				//fronta na nacitani hodnot z prevodniku
extern ULP_pins_U ULP_pins_U_global;
extern ULP_VAR_STRUCT _ULP_promenne_global;
//extern const 	PROGMEM ULP_VAR_STRUCT _ulp_flash_hodnoty;
//extern			EEMEM	ULP_VAR_STRUCT _ulp_eeprom_hodnoty;
extern volatile uint8_t ulp_OK, ULP_MUX_FLAG;

void ULP_set();
float ULP_vypocet_ppm(uint16_t Vref,uint16_t Vgas);
float ULP_Battery_check();
float ULP_Battery_check1();
int8_t ULP_start();
void ULP_init();
float ULP_Vgas_read_PPM();
float ULP_linreg(float ppm);
void hodnoty_na_LCD();
void vULP_VoltageRead(void *arg);
void vULP_PPM_read(void *arg);
esp_err_t vULP_kalibrace();


#endif /* ULP_ULP_H_ */
