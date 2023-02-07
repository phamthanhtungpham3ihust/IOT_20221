#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_config.h"
#include "dht11.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "app_mqtt.h"
#include "esp_output.h"

static const char *TAG = "NODE_RELAY";
char* check_button1_on;
char* check_button1_off;
void mqtt_data_callback(char *data, uint16_t len)
{
    check_button1_on = strstr(data, "ON1");
    check_button1_off = strstr(data, "OFF1");
    printf("%s\n", data);
    if(check_button1_on != NULL){
        esp_output_set_level(2, 1);
    }
    else if(check_button1_off != NULL){
        esp_output_set_level(2, 0);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_output_create(2);
    app_config();
    app_mqtt_init();
    app_mqtt_start();
    app_mqtt_set_cb_event(mqtt_data_callback);
}
