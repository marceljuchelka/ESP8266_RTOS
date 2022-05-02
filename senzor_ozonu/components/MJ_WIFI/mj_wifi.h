/*
 * mj_wifi.h
 *
 *  Created on: 25. 4. 2022
 *      Author: marcel
 */

#ifndef COMPONENTS_MJ_WIFI_MJ_WIFI_H_
#define COMPONENTS_MJ_WIFI_MJ_WIFI_H_

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/adc.h"
#include "lwip/err.h"
#include "lwip/sys.h"
//#include "lwip/apps/sntp.h"
//#include "sntp.h"
//#include "time.h"
#include <netdb.h>
#include "mj_wifi.h"
#include "sdkconfig.h"


/* deklarace */
#define witty  0
#if witty == 1
#define EXAMPLE_ESP_WIFI_SSID       "Svereo wifi"
#define EXAMPLE_ESP_WIFI_PASS		"svereo1122"
#define jas							3
#else
#define EXAMPLE_ESP_WIFI_SSID       "TeePee"
#define EXAMPLE_ESP_WIFI_PASS		"07006400aa"
#endif
#define EXAMPLE_ESP_MAXIMUM_RETRY  	10

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static EventGroupHandle_t s_wifi_event_group;

enum {
	vlhkost_wifi,
	teplota_wifi,
	jas_wifi,
	restart_wifi,
};

void wifi_init();

#endif /* COMPONENTS_MJ_WIFI_MJ_WIFI_H_ */
