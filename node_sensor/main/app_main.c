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

static const char *TAG = "NODE_SENSOR";

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");

struct dht11_reading dht11;
float temperature = 0.0;
float humidity = 0.0;
char data[10];

void mqtt_publish_callback(char *topic)
{
    dht11 = DHT11_read();
    if (dht11.status == DHT11_OK)
    {
        temperature = dht11.temperature;
        humidity = dht11.humidity;
        sprintf(data, "%.1f@%.1f", temperature, humidity);
        vTaskDelay(500);
        app_mqtt_publish(topic, data, 0);
    }
    else
    {
        vTaskDelay(500);
        app_mqtt_publish("data", data, 0);
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
    DHT11_init(23);
    app_config();
    app_mqtt_init();
    app_mqtt_start();
    app_mqtt_set_cb_publish(mqtt_publish_callback);
}
