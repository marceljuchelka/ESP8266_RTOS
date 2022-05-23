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
//#include "../components/MK_WIFI/mk_wifi.h"
#include "../components/MK_WIFI/mk_wifi.h"

#define MB_LED	GPIO_NUM_2




/* RTOS tools */
TaskHandle_t	BlikLedMBHandle;
TaskHandle_t	PrintFreeMemoryHandle;
TaskHandle_t	PrintOzonnaLCD;
TaskHandle_t	PrintTempHumNaLCD;
TaskHandle_t	PrintTest1;
TaskHandle_t	PrintTest2;
TaskHandle_t	PrintTime2LCD;
TaskHandle_t 	PrintToLcdHandle;

/* wifi graf parametry   https://grafy.vipro.cz/www/sensor/receive/?module=marcel&sensor[testovaci]=14.25*/
#define graf_url	"https://grafy.vipro.cz"
#define graf_modul "/www/sensor/receive/?module"
#define graf_user "marcel&sensor["
#define sen_ozon	"ozon"
#define sen_temp	"teplota"
#define sen_hum		"vlhkost"




/* parametry pro ukladani dat na flash disk   */
#define flash_data_base		0x1FC*0x1000		//adresa ulozeni dat
#define flash_sector_size	0x1000				//velikost sektoru- 4096 Byte
#define flash_addres 		flash_data_base		//


/* nastaveni webu pro zobrazovani grafu na www.vipro.cz */
typedef struct  {
	char graf_senzor_name[12];
	float graf_hodnota;
} T_GRAF_VAR;

typedef struct{
	char ssid_actual[15];
	char psw_actual[15];
}T_WIFI_PARAM;


typedef struct{
	T_GRAF_VAR data_flash;
	T_WIFI_PARAM wifi_flash;
}T_DATA_STORAGE_FLASH;

/* odeslani na web graf https://grafy.vipro.cz/www/sensor/receive/?module=marcel&sensor[testovaci]=14.25 */

esp_err_t web_send_graf(T_GRAF_VAR *hodnoty){
	char *TAG = "web send graf";
	printf("delka hodnoty %d\n",strlen(hodnoty[0].graf_senzor_name));
	char graf_buffer[218], base_buffer[64];
	sprintf(graf_buffer,"%s%s=%s", graf_url, graf_modul, graf_user);

	printf("%s\n", graf_buffer);
	for(uint8_t i = 0;i<5;i++)	{
		if(strlen(hodnoty->graf_senzor_name)) {
//			printf("hodnota %s delka stringu %d\n ",(hodnoty->graf_senzor_name), strlen(hodnoty->graf_senzor_name));
			sprintf(base_buffer,"%s%s]=%2.1f,",graf_user, hodnoty->graf_senzor_name,hodnoty->graf_hodnota);
			strcat(graf_buffer,base_buffer);
			printf("%s\n", base_buffer);
		}
		hodnoty++;
	}
	printf("%s\n", graf_buffer);
	ESP_LOGI(TAG,"test");
	return ESP_OK;
}

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


/* tisk presnz cas na LCD  */
void vPrintTimeToLcd(void *arg){
	const char *TAG = "print time LCD";
	time_t	now;
	struct tm	timeinfo = {0};
	char strftime_buf[64] = {0};
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 100;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		time(&now);
		localtime_r(&now, &timeinfo);
		strftime(strftime_buf, sizeof(strftime_buf), "%d.%m.%y  %H:%M", &timeinfo);
		ESP_LOGI(TAG, "cas %s", strftime_buf);
		if (timeinfo.tm_sec == 0) {
			I2C_TAKE_MUTEX_NORET;
			lcd_str_al(3, 0, strftime_buf, _left);
			I2C_GIVE_MUTEX_NORET;
		}
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

/*tisk ozonu na LCD */
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
		vTaskDelay(300);
	}
}

void vTeplotaVlhkostToLCD(void *arg){
	char buf[17];
	uint16_t delay;
	float	temp=0, hum=0;
	while(1){
		I2C_TAKE_MUTEX_NORET;
		temp = hdc1080_read_temp();
//		I2C_GIVE_MUTEX_NORET;
//		I2C_TAKE_MUTEX_NORET;
		hum = hdc1080_read_hum();
		I2C_GIVE_MUTEX_NORET;
		sprintf(buf, "T:%.1f%cC H:%.1f%c  ", temp, 0xDF, hum, 0x25);
//		printf("--------------- teplota a vlhkost %s\n", buf);
//		printf ("---HUM  ---- %0.6f-------\n", hum);
//		printf ("---TEMP ---- %0.6f-------\n", temp);
		if ((hum > 99.99) || (temp == 125.0)) {
			delay = 5;
		}
		else
		{
			I2C_TAKE_MUTEX_NORET;
			lcd_str_al(1, 0, buf, _left);
			I2C_GIVE_MUTEX_NORET;
			delay = 200;
		}
		vTaskDelay(delay);
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

/* ulozeni dat do flash  */
void read_data_flash(T_DATA_STORAGE_FLASH *data){
	esp_err_t esp_error;
	printf("read sizeof data %d\n", sizeof(*data));
	esp_error = spi_flash_read(flash_addres, data, sizeof(*data));
//	vTaskDelay(10);
	if(esp_error == ESP_OK){
		printf("read OK\n");
		ESP_LOGI("read flash","SSID %s  PSW %s\n",data->wifi_flash.ssid_actual,data->wifi_flash.psw_actual);
	}
	else printf("read error %s\n", esp_err_to_name(esp_error));
}
void save_data_flash(T_DATA_STORAGE_FLASH *data){
	esp_err_t esp_error;
	printf("write sizeof data %d\n", sizeof(*data));
	ESP_LOGI("write flash","SSID %s  PSW %s\n",data->wifi_flash.ssid_actual,data->wifi_flash.psw_actual);
	spi_flash_erase_sector(0x1FC);
	esp_error = spi_flash_write(flash_addres, data, sizeof(*data));
//	vTaskDelay(10);
	if(esp_error == ESP_OK) printf("save OK\n");
	else printf ("save error %s\n", esp_err_to_name(esp_error));
}

/* tisk na cely displej vse najednou  */

void vPrintToLcd (void *arg){
	float ozonPPM = 0, hum, temp;
	time_t	now;
	uint8_t delay;
	struct tm	timeinfo = {0};
	char strftime_buf[64] = {0};

	char buf_1[17], buf_2[17], buf_3[17], buf_4[17];

	while (1) {
		if (i2c_take_mutex() == ESP_OK) {
			if (xQueueReceive(OzonHandle, &ozonPPM, 10) == pdTRUE) {
				if(ozonPPM < 0) ozonPPM = 0;
//				printf("Hodnota ozonu> %f\n", ozonPPM);
				sprintf(buf_1, "OZON-> %2.1f0 PPM ", ozonPPM);
				lcd_str_al(0, 0, buf_1, _left);
			}
			xQueueReceive(TempHandle, &temp, 0);
			if (xQueueReceive(Humhandle, &hum, 0) == pdTRUE) {
				sprintf(buf_2, "T:%.1f%cC H:%.1f%c  ", temp, 0xDF, hum, 0x25);
				lcd_str_al(1, 0, buf_2, _left);
			}
			time(&now);
			localtime_r(&now, &timeinfo);
			strftime(strftime_buf, sizeof(strftime_buf), "%d.%m.%y  %H:%M:%S",
					&timeinfo);
			lcd_str_al(3, 0, strftime_buf, _left);
			i2c_give_mutex();
			delay = 70;
		} else{
			delay = 1;
			i2c_give_mutex();
		}
		vTaskDelay(delay);
	}
}


void app_main()
{
//	uart_set_baudrate(UART_NUM_0, 115000);
	vTaskDelay(20);
	printf("start\n");
	printf("*** senzor ozonu ***\n");

	OzonHandle = xQueueCreate(1,sizeof(float));
//	T_DATA_STORAGE_FLASH data_storage;
//	read_data_flash(&data_storage);
//	printf("SSID %s  PSW %s\n", data_storage.wifi_flash.ssid_actual, data_storage.wifi_flash.psw_actual);
//
//	strcpy(data_storage.wifi_flash.ssid_actual, TEST_SSID);
//	strcpy(data_storage.wifi_flash.psw_actual, TEST_PASS);
//	save_data_flash(&data_storage);
//	printf(" ram SSID %s  PSW %s\n", data_storage.wifi_flash.ssid_actual, data_storage.wifi_flash.psw_actual);
//	spi_flash_erase_sector(0x1FC);

	T_GRAF_VAR graf_data[5] = {0};
	graf_data[0].graf_hodnota = 25;
	strcpy(graf_data[0].graf_senzor_name,sen_ozon);
	graf_data[1].graf_hodnota = 30;
	strcpy(graf_data[1].graf_senzor_name,sen_temp);
	graf_data[2].graf_hodnota = 41;
	strcpy(graf_data[2].graf_senzor_name,sen_hum);
	for(uint8_t i = 0;i<5;i++){
		printf("%s  %.1f\n", graf_data[i].graf_senzor_name,graf_data[i].graf_hodnota);
	}
		web_send_graf(&graf_data[0]);
	while(1){
		;
	}

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
	mk_sntp_init(SNTP_server);




	//	printf("Referencni napeti je  %f\n", ULP_pins_U_global.Vref_U);
//	printf("napeti baterie %f\n" , ads_U_input_single(ulp_Vbat_read));



	/*spusteni tasku  */
//	xTaskCreate(vPrintFreeMemory, "printfreememory", 4096, NULL, 1, &PrintFreeMemoryHandle);
//	xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)
	xTaskCreate(vULP_VoltageRead, "voltage read", 400, NULL, 1, &VoltagereadHandle);
//	xTaskCreate(vPrintOzonNaLED, "print ozon", 2048, NULL, 1, NULL);
	xTaskCreate(vBlink_Led2, "blik led2", 1500, NULL, 1,&BlikLedMBHandle );
	xTaskCreate(vULP_PPM_read, "PPM read", 400, NULL, 1, &PPMReadHandle);
//	xTaskCreate(vPrintOzonNaLCD, "print na LCD", 2048, NULL, 1, &PrintOzonnaLCD);
//	xTaskCreate(vTeplotaVlhkostToLCD, "print temhum na LCD", 2048, NULL, 1, &PrintTempHumNaLCD);
//	xTaskCreate(vText1, "print test 1", 2048, NULL, 2, &PrintTest1);
//	xTaskCreate(vPrintTimeToLcd, "print time LCD", 2048, NULL, 2, &PrintTime2LCD);
	xTaskCreate(hdc1080_read, "cteni temp a hum pravidelne", 400, NULL, 1, &hdc1080Task);
	xTaskCreate(vPrintToLcd, "Print na LCD", 1600, NULL, 1, &PrintToLcdHandle);
	while(1){
		vTaskDelay(100);
		ESP_LOGI("Free Mem:","%d\n", esp_get_free_heap_size());
		printf("vULP voltage read: %d \n", uxTaskGetStackHighWaterMark(VoltagereadHandle));
		printf("vULP PPM read: %d \n", uxTaskGetStackHighWaterMark(PPMReadHandle));
		printf("HDC1080read: %d \n", uxTaskGetStackHighWaterMark(hdc1080Task));
		printf("Print to lcd: %d \n", uxTaskGetStackHighWaterMark(PrintToLcdHandle));
		printf("Blikled: %d \n", uxTaskGetStackHighWaterMark(BlikLedMBHandle));



//		esp_task_wdt_reset();
	}


}
