/* test preruseni od GPIO
 * vyuzivam ho na nacitani IR signalu
*/
#include <stdio.h>
#include <string.h>
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "FreeRTOSConfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "portmacro.h"
#include "sdkconfig.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "../components/ADS_1115/ads_1115.h"
#include "../components/ULP/ulp.h"
#include "../components/TM_1637_LED/tm_1637_led.h"
#include "../components/MK_LCD/mk_lcd44780.h"
#include "../components/MK_I2C/mk_i2c.h"
#include "../components/MJ_HDC1080/hdc1080.h"
//#include "../MJ_WIFI/mj_wifi.h"
#include "../components/MK_WIFI/mk_wifi.h"

#define MB_LED	GPIO_NUM_2

TaskHandle_t	BlikLedMBHandle;
TaskHandle_t	PrintFreeMemoryHandle;
TaskHandle_t	PrintOzonnaLCD;
TaskHandle_t	PrintTempHumNaLCD;
TaskHandle_t	PrintTest1;
TaskHandle_t	PrintTest2;




void vBlink_Led2(void *arg){
	gpio_config_t gpio_cfg;
	gpio_cfg.pin_bit_mask = (1<<MB_LED);
	gpio_cfg.mode = GPIO_MODE_DEF_OUTPUT;
	gpio_config(&gpio_cfg);
	uint8_t uroven=0;

	while(1){
		gpio_set_level(GPIO_NUM_2, uroven=uroven^1);
		vTaskDelay(100);
	}
}



void mk_got_ip_cb( char * ip ) {

	tcpip_adapter_ip_info_t ip_info;
	tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_STA, &ip_info );


	printf( "[STA] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[STA] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[STA] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );


	tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_AP, &ip_info );


	printf( "[AP] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[AP] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[AP] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );

}

void mk_sta_disconnected_cb( void ) {

	ESP_LOGE("wifi","disconect");


}

void mk_ap_join_cb( char * mac ) {

}

void mk_ap_leave_cb( char * mac ) {

}


void print_PPM(void *arg){
	while(1){
		while(1){
			vULP_PPM_read(0);
		vTaskDelay(200);
		}
	}
}

void vPrintOzonNaLED(void *arg) {
	float ozonPPM = 0;
	char OzonBuf[5];
	while (1) {
		if (xQueuePeek(OzonHandle, &ozonPPM, 10) == pdTRUE) printf("Hodnota ozonu> %f\n", ozonPPM);
//		if (ozonPPM<10)
//			ozonPPM = ozonPPM;
			sprintf(OzonBuf,"P-%2d", (int)ozonPPM);
//			printf("ozon string> %s strlen: %d\n", OzonBuf, strlen(OzonBuf));
			led_print(0, "    ");
			led_print(4-strlen(OzonBuf), OzonBuf);
		vTaskDelay(100);
	}
}

void vPrintOzonNaLCD(void *arg){
	float ozonPPM = 0;
	char horni_buf[17];

	while(1){
		if (xQueuePeek(OzonHandle, &ozonPPM, 10) == pdTRUE){
			printf("Hodnota ozonu> %f\n", ozonPPM);
			sprintf(horni_buf,"PPM OZON-> %2.1f0", ozonPPM);
			I2C_TAKE_MUTEX_NORET;
			lcd_str_al(0, 0, horni_buf, _left);
			I2C_GIVE_MUTEX_NORET;
		}
		vTaskDelay(200);
	}
}

void vTeplotaVlhkostToLCD(void *arg){
	char buf[17];
	while(1){
		sprintf(buf,"T:%.1f%cC H:%.1f%c  ",hdc1080_read_temp(),0xDF,hdc1080_read_hum(),0x25);
		I2C_TAKE_MUTEX_NORET;
		lcd_str_al(1, 0, buf, _left);
		I2C_GIVE_MUTEX_NORET;
		printf("%s\n",buf);
		vTaskDelay(500);
	}
}

//void vTest_print2(void *arg){
//	ESP_LOGI("print2","created");
//	while (1) {
////		vTaskDelay(20);
//		ESP_LOGI("print2","while");
//		printf("print test2\n");
//		ESP_LOGI("print2","kill");
//		vTaskDelete(PrintTest2);
//		vTaskDelay(50);
//	}
//}
//
//void vTest_print1(void *arg) {
//	while (1) {
//		printf("print test1\n");
//		vTaskDelay(50);
//		xTaskCreate(vTest_print2, "print test2", 2048, NULL, 1, &PrintTest2);
//		vTaskDelay(50);
////		ESP_LOGI("print2", "deleted");
////		vTaskDelete(PrintTest2);
//
//	}
//}

void vText2(void *arg) {
	ESP_LOGI("print2","created");
	while (1) {
		vTaskDelay(300 / portTICK_PERIOD_MS);
		printf("Text2 \n");
		ESP_LOGI("print2","kill");
		vTaskDelete(PrintTest2);
	}
}


void vText1(void *arg) {
	while (1) {
		printf("Text1 \n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		xTaskCreate(vText2, "text2", 2048, NULL, 1, &PrintTest2);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
//		ESP_LOGI("print2","kill");
//		vTaskDelete(PrintTest2);
	}
}

void vPrintFreeMemory(void *arg) {
	while (1) {
		ESP_LOGI("Free Mem:","%d\n", esp_get_free_heap_size());
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void app_main()
{
	vTaskDelay(200);
	printf("start\n");
	printf("*** senzor ozonu ***\n");

	OzonHandle = xQueueCreate(1,sizeof(float));

//	my_i2c_config();
	i2c_init(I2C_NUM_0, I2C_SCL_PIN, I2C_SDA_PIN);
	vTaskDelay(10);
	ULP_init();
	vTaskDelay(10);
	vULP_kalibrace();
	tm_1637_gpio_init();
	vTaskDelay(10);
	lcd_init();
	vTaskDelay(10);
	hdc1080_init();
	led_day_set();
	led_print(0, "1234");
	lcd_str("start");
	vTaskDelay(100);
	nvs_flash_init();
	mk_wifi_init(WIFI_MODE_STA, mk_got_ip_cb, mk_sta_disconnected_cb, NULL,NULL);

	//	printf("Referencni napeti je  %f\n", ULP_pins_U_global.Vref_U);
//	printf("napeti baterie %f\n" , ads_U_input_single(ulp_Vbat_read));



	/*spusteni tasku  */
//	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 1, &PrintFreeMemoryHandle);
//	xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)
	xTaskCreate(vULP_VoltageRead, "voltage read", 1500, NULL, 1, &VoltagereadHandle);
//	xTaskCreate(vPrintOzonNaLED, "print ozon", 2048, NULL, 1, NULL);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1,&BlikLedMBHandle );
	xTaskCreate(vULP_PPM_read, "PPM read", 1500, NULL, 1, &PPMReadHandle);
	xTaskCreate(vPrintOzonNaLCD, "print na LCD", 2048, NULL, 1, &PrintOzonnaLCD);
	xTaskCreate(vTeplotaVlhkostToLCD, "print temhum na LCD", 2048, NULL, 1, &PrintTempHumNaLCD);
	xTaskCreate(vText1, "print test 1", 2048, NULL, 1, &PrintTest1);

	while(1){
		vTaskDelay(100);
		ESP_LOGI("Free Mem:","%d\n", esp_get_free_heap_size());
	}


}
