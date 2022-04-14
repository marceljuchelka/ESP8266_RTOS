/*
 * ulp.c
 *
 *  Created on: 17. 5. 2020
 *      Author: Marcel Juchelka
 *      senzor ULP pro O3 a mereni pomoci ads1115 I2C AD prevodniku
  */



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "math.h"
#include "../ADS_1115/ads_1115.h"
#include "../ULP/ulp.h"
//#include "ADS_1115/ads_1115.h"

ULP_VAR_STRUCT _ULP_promenne_global = {.M_span = M_span_def,.sens_code = Sens_code_def,.PPM_select = PPM_sel_def};


ULP_pins_U ULP_pins_U_global;

volatile uint8_t ulp_OK = 0, ULP_MUX_FLAG = 0;

/*inicializace ULP I2C*/
void ULP_init(){															//nastaveni prevodniku s O3 senzorem
	ads_i2c_address = ULP_ADS_address;
	if(ads_test_address(ads_i2c_address) == ESP_OK) {									//testovani je li prevodnik
		ulp_OK = 1;															//prevodnik je na adrese
		ads_write_register(ADS_Config_register,0x8583);						//reset config
		Buf_Config_register = ads_read_register(ADS_Config_register);		//nacte config register do Buf_Config_register
//		lcd_bin_al(0,0,Buf_Config_register,16,_left);
//		ads_set_mux(ADS_MUX4);												//nastavi 100 : AINP = AIN0 and AINN = GND
		ads_set_datarate(ADS_DR8);											//100 : 8 SPS
		ads_set_gain(ADS_FSR1);												//001 : FSR = ±4.096 V
		ads_bit_set((ADS_MODE),ADS_Single);									//single or Continuous-conversion mode
		Buf_Config_register = ads_read_register(ADS_Config_register);
		ULP_pins_U_global.Vref_U = ads_read_single_mux(ulp_Vref_read) * ads_fsr_table[(Buf_Config_register>>ADS_PGA0) & 0X07];
		printf("v ref> %f", ULP_pins_U_global.Vref_U);
	}
	else{
		printf("modul ULC chybi na adrese %x", ULP_ADS_address);
//	lcd_cls();
//	lcd_str_P(PSTR("modul ULC chybi"));
//	lcd_str_al_P(1,0,PSTR("na adrese H"),_left);
//	lcd_hex(ULP_ADS_address);
//	_delay_ms(2000);
	}
}


void ULP_set(){									//vyber ktery modul budeme pouzivat
	ads_i2c_address = ULP_ADS_address;
	Buf_Config_register = ads_read_register(ADS_Config_register);		//nacte config register do Buf_Config_register
}

void hodnoty_na_LCD(){
//	lcd_init();
//	copy_eemem2ram(&ULP_promenne_global, &_ulp_eeprom_hodnoty, sizeof(_ulp_eeprom_hodnoty));
//	ULP_promenne_global.sens_code = Sens_code_def;
//	ULP_promenne_global.M_span=(((float)ULP_promenne_global.sens_code/(float)1000) * (float)499)/(float)1000;				//vypocet mV/PPM
//	lcd_str_P(PSTR("Mspan     Sens C"));
//	lcd_float(1,0,ULP_promenne_global.M_span,2,_left);
//	lcd_float(1,15,ULP_promenne_global.sens_code,2,_right);
//	_delay_ms(2000);
//	lcd_cls();
//	lcd_str_P(PSTR("Baterie Volt"));
//	lcd_float(1,0,ULP_Battery_check1(),2,_left);
//	_delay_ms(2000);
	ULP_start();
//	lcd_cls();
//	lcd_str_P(PSTR("Vref        Vgas"));
//	lcd_float(1,15,ULP_pins_U_global.Vgas_U,2,_right);
//	lcd_float(1,0,ULP_pins_U_global.Vref_U,2,_left);
//	_delay_ms(2000);
//	lcd_cls();
//	lcd_str_P(PSTR("Voffset      PPM"));
//	lcd_float(1,15,ULP_Vgas_read_PPM(),2,_right);
//	lcd_float(1,0,ULP_pins_U_global.Voffset_U,2,_left);
//	_delay_ms(2000);
}


/*vypocet PPM*/
float ULP_vypocet_ppm(uint16_t Vref,uint16_t Vgas){
float PPM;
PPM = ((float)Vref - (float)ULP_pins_U_global.Voffset_U - (float)Vgas)/_ULP_promenne_global.M_span;
//return ((float)_Vref - (float)Vgas)/M_spam;
PPM = round(PPM*100)/100;
if(PPM < 0) PPM = 0;
return PPM;
}


/*kontrola baterie*/
float ULP_Battery_check(){										//kontrola baterie je li pod 1400 (2.8V) vraci -1
	ULP_pins_U napeti;
	napeti.Vref_U = ads_U_input_single(ulp_Vref_read);					//napeti Vgas v mV
	if(napeti.Vref_U < U_Bat_min) return ulp_error;
return (napeti.Vref_U*2)/1000;
}
float ULP_Battery_check1(){										//kontrola baterie je li pod Vbat_min (2.8V) vraci -1
	float Ubattery;
	Ubattery = ads_U_input_single(ulp_Vbat_read);					//napeti Baterie v mV
	if(Ubattery < U_Bat_min) return ulp_error;
return Ubattery;
}

void ULP_set_cont(void *arg){
	ads_set_mux(ulp_Vgas_read);
	ads_bit_set((ADS_MODE),ADS_Continuous_mode);						//single or Continuous-conversion mode
}

void vULP_PPM_read(void *arg){
	float PPM;
	ULP_pins_U_global.Vgas_U = ads_read_register(ADS_Conversion_register) * ads_fsr_table[(Buf_Config_register>>ADS_PGA0) & 0X07];
	PPM = (ULP_pins_U_global.Vref_U - ULP_pins_U_global.Voffset_U - ULP_pins_U_global.Vgas_U)/_ULP_promenne_global.M_span;
	printf("Vgas     Vref     Vofset\n");
	printf("%f       %f       %f \n\n", ULP_pins_U_global.Vgas_U, ULP_pins_U_global.Vref_U, ULP_pins_U_global.Voffset_U);
	printf("----------PPM %f\n\n",PPM);

}


/* nacitani hodnot po startu*/
int8_t ULP_start(){
//	ULP_pins_U pracovni;
//	float VGas_prumer = 0,Vref_prumer = 0;
//	uint8_t i;
//	/*nacitani referencniho napeti a prumerovani */
//	for(i=0;i<5;i++){
//		Vref_prumer = Vref_prumer+ads_U_input_single(ulp_Vref_read);
//	}
//	ULP_pins_U_global.Vref_U = Vref_prumer/i;
//
//	while(1){
//		/*nacitani Vgas napeti a prumerovani */
//		for(i = 0;i<5;i++){
//		VGas_prumer = VGas_prumer + ads_U_input_single(ulp_Vgas_read);
//		}
//		ULP_pins_U_global.Vgas_U = VGas_prumer/i;
//		ULP_pins_U_global.Voffset_U = ULP_pins_U_global.Vref_U - ULP_pins_U_global.Vgas_U;
//		if(klav_OK) vypis_hodnot_mV();
//		if(ULP_pins_U_global.Voffset_U < 0){	// je li po startu negativni offset tak pust ozon a opakuj mereni
//			ozon_ON;
//			if(klav_OK) lcd_str_al_P(0, 15, "O", _left);
//			_delay_ms(2000);
//			ozon_OFF;
//			if(klav_OK) lcd_str_al_P(0, 15, "0", _left);
//			_delay_ms(4000);
//			VGas_prumer = 0;					//nulovani vgas v pripade zaporneho cisla - opakovani
//		}
//		else break;
//	}
////	ULP_pins_U_global = pracovni;
	return ulp_enable;
}

float ULP_Vgas_read_PPM(){
	float PPM;
//	ULP_pins_U_global.Vref_U = ads_U_input_single(ulp_Vref_read);
	PPM = (ULP_pins_U_global.Vref_U - ads_U_input_single(ulp_Vgas_read))/_ULP_promenne_global.M_span;
	if(PPM<0)PPM = 0;
	return PPM;
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
