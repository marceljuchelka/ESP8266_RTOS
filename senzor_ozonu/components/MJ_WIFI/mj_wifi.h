/*
 * mj_wifi.h
 *
 *  Created on: 25. 4. 2022
 *      Author: marcel
 */

#ifndef COMPONENTS_MJ_WIFI_MJ_WIFI_H_
#define COMPONENTS_MJ_WIFI_MJ_WIFI_H_

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

#endif /* COMPONENTS_MJ_WIFI_MJ_WIFI_H_ */
