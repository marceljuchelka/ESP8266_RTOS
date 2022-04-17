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
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "math.h"
#include "../ADS_1115/ads_1115.h"
#include "../ULP/ulp.h"
//#include "ADS_1115/ads_1115.h"

ULP_VAR_STRUCT _ULP_promenne_global = {.M_span = M_span_def,.sens_code = Sens_code_def,.PPM_select = PPM_sel_def};
ULP_pins_U ULP_pins_U_global;

QueueHandle_t fronta_vzorku_napeti;				//fronta na nacitani hodnot z prevodniku
QueueHandle_t	OzonHandle;						//aktualni hodnota ozonu


volatile uint8_t ulp_OK = 0, ULP_MUX_FLAG = 0;

/* ******************************************************RTOS  *********************************/
TaskHandle_t	PPMReadHandle;
TaskHandle_t 	voltagereadHandle;


void ULP_set_cont(void *arg){
	ads_set_mux(ulp_Vgas_read);
	ads_bit_set((ADS_MODE),ADS_Continuous_mode);						//single or Continuous-conversion mode
}

/* nacitani hodnot ze senzoru  	xTaskCreate(vULP_VoltageRead, "voltage read", 1300, NULL, 1, voltagereadHandle); */
void vULP_VoltageRead(void *arg){
	OzonHandle = xQueueCreate(1,sizeof(float));							//aktualni hodnota ozonu
	fronta_vzorku_napeti = xQueueCreate(5,sizeof(float));				//fronta nacitani hodnot y cidla
	ULP_set_cont(0);
	float nacteno = 0;
	while(1){
		nacteno = ads_read_register(ADS_Conversion_register) * ads_fsr_table[(Buf_Config_register>>ADS_PGA0) & 0X07];  	//nacteni hodnoty s pinu Vgas a nasobeni koeficientem dle tabulky
//		ESP_LOGI("VoltRead","nacitani pamet k uvolneni %d", uxTaskGetStackHighWaterMark(NULL));
		if(xQueueSendToBack(fronta_vzorku_napeti,&nacteno,0)!=pdTRUE) {
			vTaskResume(PPMReadHandle);
		}
		esp_task_wdt_reset();
		vTaskDelay(20);
	}
}

/* nacteni a vypocet PPM  ulozeni do ozon queue xTaskCreate(vULP_PPM_read, "PPM read", 1300, NULL, 1, &PPMReadHandle)*/
void vULP_PPM_read(void *arg) {

	float PPM, DataZFronty, prumer = 0;
	while (1) {
		while (xQueueReceive(fronta_vzorku_napeti, &DataZFronty, 10)) {
			prumer = prumer + DataZFronty;
		};
		ULP_pins_U_global.Vgas_U = prumer/5;
		PPM = (ULP_pins_U_global.Vref_U - ULP_pins_U_global.Voffset_U - ULP_pins_U_global.Vgas_U) / _ULP_promenne_global.M_span;
		xQueueOverwrite(OzonHandle,&PPM);
		prumer = 0;
		vTaskSuspend(NULL);
//		ESP_LOGI("PPM read", "zbyvajici pamet %d\n",uxTaskGetStackHighWaterMark(NULL));

	}
}

/*inicializace ULP I2C */
void ULP_init(){															//nastaveni prevodniku s O3 senzorem

	ads_i2c_address = ULP_ADS_address;
	if(ads_test_address(ads_i2c_address) == ESP_OK) {									//testovani je li prevodnik
		ulp_OK = 1;															//prevodnik je na adrese
		ads_write_register(ADS_Config_register,0x8583);						//reset config
		Buf_Config_register = ads_read_register(ADS_Config_register);		//nacte config register do Buf_Config_register
		ads_set_datarate(ADS_DR8);											//100 : 8 SPS
		ads_set_gain(ADS_FSR1);												//001 : FSR = ±4.096 V
		ads_bit_set((ADS_MODE),ADS_Single);									//single or Continuous-conversion mode
		Buf_Config_register = ads_read_register(ADS_Config_register);
		ULP_pins_U_global.Vref_U = ads_read_single_mux(ulp_Vref_read) * ads_fsr_table[(Buf_Config_register>>ADS_PGA0) & 0X07];
		printf("v ref> %f", ULP_pins_U_global.Vref_U);
	}
	else{
		printf("modul ULC chybi na adrese %x", ULP_ADS_address);
	}
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

