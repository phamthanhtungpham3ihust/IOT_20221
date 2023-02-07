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
httpd_req_t *REG;
json_gen_test_result_t result;

char temp[10];
char humi[10];
char REQUEST[352];
char REQUEST_GET[352];
char REQUEST_BUTTON[352];
char SUBREQUEST[200];
char SUBREQUEST_BUTTON[200];
char data[10];
char data_get_dht11[100];
int flag = 0;
int button;
char tmp[600];
char *check_on_button_1;
char *check_on_button_2;
char *check_off;


esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}


static void post_rest_function(void)
{
    esp_http_client_config_t config_post = {
        .url = URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    if(flag == 1)
    {
        json_gen_float_button(&result, "button", 1.0, REQUEST_BUTTON);
        flag = -1;
    }
    else if(flag == 0)
    {
        json_gen_float_button(&result, "button", 0.0, REQUEST_BUTTON);
        flag = -1;
    }
    else
    {
        json_gen_float_button(&result, "button", -1.0, REQUEST_BUTTON);
        flag = -1;
    }
    esp_http_client_set_post_field(client, REQUEST_BUTTON, strlen(REQUEST_BUTTON));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

esp_err_t client_event_get_button1_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        //sprintf(tmp, "HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        check_on_button_1 = strstr((char*)evt->data, "1.0");
        //check_off = strstr((char*)evt->data, "0.0");
        if(check_on_button_1 != NULL)
        {
            button = 1;
            //esp_output_set_level(2, 1);
            app_mqtt_publish("button1", "ON1", 0);
        }
        else
        {
            button = 0;
            //esp_output_set_level(2, 0);
            app_mqtt_publish("button1", "OFF1", 0);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

esp_err_t client_event_get_button2_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        //sprintf(tmp, "HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        check_on_button_2 = strstr((char*)evt->data, "1.0");
        //check_off = strstr((char*)evt->data, "0.0");
        if(check_on_button_2 != NULL)
        {
            //button = 1;
            //esp_output_set_level(2, 1);
            app_mqtt_publish("button2", "ON2", 0);
        }
        else
        {
            //button = 0;
            //esp_output_set_level(2, 0);
            app_mqtt_publish("button2", "OFF2", 0);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

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
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

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
        //sprintf(REQUEST, "POST %s HTTP/1.1\nHost: %s\nContent-Length: %d\n\n%s\n", HOST, WEB_SERVER, strlen(SUBREQUEST), SUBREQUEST);
        
        esp_http_client_set_post_field(client, REQUEST, strlen(REQUEST));
        esp_http_client_set_header(client, "Content-Type", "application/json");

        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
}

void mqtt_data_event_callback(char *data, uint16_t len)
{
    for (int i = 0; i < 4; i++)
    {
        temp[i] = data[i];
        humi[i] = data[i+5];
    }
}

void http_get_dht11_callback(httpd_req_t *req)
{
    sprintf(data_get_dht11, "{\"temperature\": \"%s\",\"humidity\": \"%s\", \"button\": \"%d\"}", temp, humi, button);
    REG = req;
    httpd_resp_send(req, data_get_dht11, strlen(data_get_dht11));
    //printf("%s\n", data_get_dht11);
}

void http_switch1_callback(char *buf, int len)
{
    if(*buf == '1')
    {
        button = 1;
        flag = 1;
        printf("ON\n");
        post_rest_function();
        esp_output_set_level(2, 1);
        app_mqtt_publish("button1", "ON1", 0);
    }
    else
    {
        button = 0;
        flag = 0;
        printf("OFF\n");
        post_rest_function();
        esp_output_set_level(2, 0);
        app_mqtt_publish("button1", "OFF1", 0);
    }
}

void app_main(void)
{
    // Inittlize flash
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
    app_mqtt_set_cb_event(mqtt_data_event_callback);
    start_webserver();
    http_get_dhtt11_set_callback(http_get_dht11_callback);
    http_switch1_set_callback(http_switch1_callback);
    
    xTaskCreate(&http_get_button1_task, "http_get_task", 4096, NULL, 8, NULL);
    xTaskCreate(&http_get_button2_task, "http_get_task", 4096, NULL, 7, NULL);
    //xTaskCreate(&http_post_data_task, "http_post_data_task", 4096, NULL, 6, NULL);
    xTaskCreate(&http_post_task, "http_post_data_task", 4096, NULL, 6, NULL);
}
