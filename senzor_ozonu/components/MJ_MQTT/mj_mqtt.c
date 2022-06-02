/*
 * mj_mqtt.c
 *
 *  Created on: 30. 5. 2022
 *      Author: marcel
 */



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mqtt_client.h"
#include "mqtt_config.h"
#include "mqtt_supported_features.h"
#include "esp_log.h"



const esp_mqtt_client_config_t mqtt_cfg = {
			.username 	= "Marcel",
			.password 	= "xer1",
//			.host 		= "test.mosquitto.org",
//			.port		=	1883,
//			.lwt_topic	= "command",
//			.lwt_msg	= "teplota",
//			.lwt_msg_len=	sizeof(mqtt_cfg.lwt_msg),
//			.skip_cert_common_name_check = 0,
			.uri		=	"mqtt://test.mosquitto.org:1883",
	};

esp_mqtt_client_handle_t client;


esp_mqtt_event_id_t 		mqtt_event;







void vMQTT_send_message_task(void *arg){
//	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_publish(client, "command", "test data", 12, 0, 0);
}

void mqtt_config(){
	const char *TAG = "mqtt_config";
	client = esp_mqtt_client_init(&mqtt_cfg);
//	ESP_LOGI(TAG,"client register event %d\n",esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client));
	ESP_LOGI(TAG,"client start %d\n",esp_mqtt_client_start(client));

}






