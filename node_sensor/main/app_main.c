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

struct dht11_reading dht11;
float temperature = 0.0;
float humidity = 0.0;
char data[10];

void mqtt_publish_callback(char *topic)
{
    // Reading DHT11 data
    dht11 = DHT11_read();

    // If checksum is correct, temparature and humidity data are updated
    if (dht11.status == DHT11_OK)
    {
        temperature = dht11.temperature;
        humidity = dht11.humidity;
        sprintf(data, "%.1f@%.1f", temperature, humidity);
        printf("%s\n", data);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
        app_mqtt_publish(topic, data, 0);
    }
    // If checksum isn't correct, publishing last temparature and humidity data
    else
    {
        vTaskDelay(15000/ portTICK_PERIOD_MS);
        app_mqtt_publish("data", data, 0);
        printf("Failed\n");
    }
    
}

void app_main(void)
{
    // Initialize flash
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize DHT11 data pin at pin 23
    DHT11_init(23);

    // Initialize Wi-Fi
    app_config();

    // Initialize MQTT protocol
    app_mqtt_init();
    app_mqtt_start();

    // Set callback function when having mqtt data publishing event
    app_mqtt_set_cb_publish(mqtt_publish_callback);
}
