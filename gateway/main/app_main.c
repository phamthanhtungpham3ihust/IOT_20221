#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "app_config.h"
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
#include "app_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "json_generator.h"
#include "esp_output.h"

#define WEB_SERVER "industrial.api.ubidots.com"
#define URL_POST "http://industrial.api.ubidots.com/api/v1.6/devices/ESP32_DATA?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define HOST "/api/v1.6/devices/ESP32_DATA?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define URL_GET "https://industrial.api.ubidots.com/api/v1.6/devices/esp32_data/button/lv/?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define URL_GET_BUTTON2 "https://industrial.api.ubidots.com/api/v1.6/devices/esp32_data/button_1/lv/?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define GET     "/api/v1.6/devices/esp32_data/Button/?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define WEB_PORT "80"

static const char *TAG = "GATEWAY";
json_gen_test_result_t result;

char temp[10];
char humi[10];
char REQUEST[352];
char REQUEST_GET[352];
char REQUEST_BUTTON[352];
char REQUEST_BUTTON2[352];

char data[10];
char data_get_dht11[100];
int flag1 = 0;
int flag2 = 0;
int button1 = 0;

int button2 = 0;

char tmp[600];

char *check_on_button_1;
char *check_on_button_2;


// Callback function when receiving respone of temparature and humidity post request
esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    return ESP_OK;
}

// Callback function when receiving respone of button1 post request
esp_err_t client_event_post_button1_handler(esp_http_client_event_handle_t evt)
{
    return ESP_OK;
}

// Callback function when receiving respone of button2 post request
esp_err_t client_event_post_button2_handler(esp_http_client_event_handle_t evt)
{
    return ESP_OK;
}

// Function that posts status of button1 to Ubidots Clound
static void post_button1(void)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_button1_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    if(flag1 == 1)
    {
        json_gen_float_button(&result, "button", 1.0, REQUEST_BUTTON);
        flag1 = -1;
    }
    else if(flag1 == 0)
    {
        json_gen_float_button(&result, "button", 0.0, REQUEST_BUTTON);
        flag1 = -1;
    }
    else
    {
        json_gen_float_button(&result, "button", -1.0, REQUEST_BUTTON);
        flag1 = -1;
    }
    esp_http_client_set_post_field(client, REQUEST_BUTTON, strlen(REQUEST_BUTTON));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

// Function that posts status of button2 to Ubidots Clound
static void post_button2(void)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_button2_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    if(flag2 == 1)
    {
        json_gen_float_button(&result, "button_1", 1.0, REQUEST_BUTTON2);
        flag2 = -1;
    }
    else if(flag2 == 0)
    {
        json_gen_float_button(&result, "button_1", 0.0, REQUEST_BUTTON2);
        flag2 = -1;
    }
    else
    {
        json_gen_float_button(&result, "button_1", -1.0, REQUEST_BUTTON2);
        flag2 = -1;
    }
    esp_http_client_set_post_field(client, REQUEST_BUTTON2, strlen(REQUEST_BUTTON2));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}


// Callback function when receiving respone of button1 get request
esp_err_t client_event_get_button1_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        //printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        check_on_button_1 = strstr((char*)evt->data, "1.0");
        if(check_on_button_1 != NULL)
        {
            button1 = 1;
            app_mqtt_publish("button1", "ON1", 0); 
        }
        else
        {
            button1 = 0;
            app_mqtt_publish("button1", "OFF1", 0);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Callback function when receiving respone of button2 get request
esp_err_t client_event_get_button2_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        check_on_button_2 = strstr((char*)evt->data, "1.0");
        if(check_on_button_2 != NULL)
        {
            button2 = 1;
            app_mqtt_publish("button2", "ON2", 0);
        }
        else
        {
            button2 = 0;
            app_mqtt_publish("button2", "OFF2", 0);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}


// Callback function when receiving mqtt data event
void mqtt_data_event_callback(char *data, uint16_t len)
{
    for (int i = 0; i < 4; i++)
    {
        temp[i] = data[i];
        humi[i] = data[i+5];
    }
}


// Task that gets status of button1 from Ubidots GUI
static void http_get_button1_task(void *pvParameters)
{
    for( ;; )
    {
        esp_http_client_config_t config_get = {
        .url = URL_GET,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_button1_handler};
        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Task that gets status of button2 from Ubidots GUI
static void http_get_button2_task(void *pvParameters)
{
    for( ;; )
    {
        esp_http_client_config_t config_get = {
        .url = URL_GET_BUTTON2,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_button2_handler};
        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}


// Task that sends temparature and humidity data to Ubidots Clound by http post method
static void http_post_task(void)
{
    for( ;;)
    {
        esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
        esp_http_client_handle_t client = esp_http_client_init(&config_post);
        json_gen_string_temp_humidity(&result, "temperature", temp, "humidity", humi, REQUEST);
        
        esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
        esp_http_client_set_header(client, "Content-Type", "application/json");

        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
}

// Callback function when receiving get request from web browser
void http_get_dht11_callback(httpd_req_t *req)
{
    sprintf(data_get_dht11, "{\"temperature\": \"%s\",\"humidity\": \"%s\", \"button1\": \"%d\", \"button2\": \"%d\"}", temp, humi, button1, button2);
    httpd_resp_send(req, data_get_dht11, strlen(data_get_dht11));
    //printf("%s\n", data_get_dht11);
}

// Callback function when receiving button1 event from web browser
void http_switch1_callback(char *buf, int len)
{
    if(*buf == '1')
    {
        button1 = 1;
        flag1 = 1;
        printf("ON\n");
        post_button1();
        esp_output_set_level(2, 1);
        app_mqtt_publish("button1", "ON1", 0);
    }
    else
    {
        button1 = 0;
        flag1 = 0;
        printf("OFF\n");
        post_button1();
        esp_output_set_level(2, 0);
        app_mqtt_publish("button1", "OFF1", 0);
    }
}

// Callback function when receiving button2 event from web browser
void http_switch2_callback(char *buf, int len)
{
    if(*buf == '1')
    {
        button2 = 1;
        flag2 = 1;
        post_button2();
        //esp_output_set_level(2, 1);
        app_mqtt_publish("button2", "ON2", 0);
    }
    else
    {
        button2 = 0;
        flag2 = 0;
        post_button2();
        //esp_output_set_level(2, 0);
        app_mqtt_publish("button2", "OFF2", 0);
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

    esp_output_create(2);

    // Initialize Wi-Fi
    app_config();

    // Initialize MQTT protocol
    app_mqtt_init();
    app_mqtt_start();

    // Start WebServer
    start_webserver();

    // Set callback function for MQTT event data
    app_mqtt_set_cb_event(mqtt_data_event_callback);
    
    // Set callback function for http get request from browser
    http_get_dhtt11_set_callback(http_get_dht11_callback);
    
    // Set callback function for button1 event request from browser
    http_switch1_set_callback(http_switch1_callback);

    // Set callback function for button1 event request from browser
    http_switch2_set_callback(http_switch2_callback);

    // Creat task getting status of button1 on Ubidots GUI
    xTaskCreate(&http_get_button1_task, "http_get_task", 4096, NULL, 8, NULL);
    
    // Creat task getting status of button2 on Ubidots GUI
    xTaskCreate(&http_get_button2_task, "http_get_task", 4096, NULL, 8, NULL);

    // Creat task sending temparature and humidity data to Ubidots Clound by http post method
    xTaskCreate(&http_post_task, "http_post_data_task", 4096, NULL, 9, NULL);
}
