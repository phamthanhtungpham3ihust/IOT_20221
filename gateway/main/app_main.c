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
#define GET     "/api/v1.6/devices/esp32_data/Button/?token=BBFF-ZaK8DyPbwLgHe0vB1QGSPZXEonwkLx"
#define WEB_PORT "80"

static const char *TAG = "GATEWAY";

json_gen_test_result_t result;
float state_get = 0.0;
float state = 0.0;
char temp[10];
char humi[10];
char REQUEST[352];
char REQUEST_GET[352];
char REQUEST_BUTTON[352];
char SUBREQUEST[200];
char SUBREQUEST_BUTTON[200];
char data[10];
char data_get_dht11[60];
int flag = 0;
char tmp[600];
char *check_on;
char *check_off;
char *error;

static void http_post_data_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s;
    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            printf("ERROR from post task\n");
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            printf("ERROR from post task\n");
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);
        json_gen_string_temp_humidity(&result, "temperature", temp, "humidity", humi, SUBREQUEST);
        sprintf(REQUEST, "POST %s HTTP/1.1\nHost: %s\nContent-Length: %d\n\n%s\n", HOST, WEB_SERVER, strlen(SUBREQUEST), SUBREQUEST);
        
        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 1;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(40 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");
    
        close(s);
        for(int countdown = 20; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

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

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        sprintf(tmp, "HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        check_on = strstr(tmp, "1.0");
        check_off = strstr(tmp, "0.0");
        if(check_on != NULL)
        {
            esp_output_set_level(2, 1);
            app_mqtt_publish("button", "ON", 0);
        }
        else if(check_off != NULL)
        {
            esp_output_set_level(2, 0);
            app_mqtt_publish("button", "OFF", 0);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void http_get_task(void *pvParameters)
{
    for( ;; )
    {
        esp_http_client_config_t config_get = {
        .url = URL_GET,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_handler};
        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_http_client_perform(client);
        esp_http_client_cleanup(client);
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
    sprintf(data_get_dht11, "{\"temperature\": \"%s\",\"humidity\": \"%s\"}", temp, humi);
    httpd_resp_send(req, data_get_dht11, strlen(data_get_dht11));
    printf("%s\n", data_get_dht11);
}

void http_switch1_callback(char *buf, int len)
{
    if(*buf == '1')
    {
        flag = 1;
        printf("ON\n");
        post_rest_function();
        esp_output_set_level(2, 1);
        app_mqtt_publish("button", "ON", 0);
    }
    else
    {
        flag = 0;
        printf("OFF\n");
        post_rest_function();
        esp_output_set_level(2, 0);
        app_mqtt_publish("button", "OFF", 0);
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
    app_mqtt_set_cb_event(mqtt_data_event_callback);
    start_webserver();
    http_get_dhtt11_set_callback(http_get_dht11_callback);
    http_switch1_set_callback(http_switch1_callback);
    
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    xTaskCreate(&http_post_data_task, "http_post_data_task", 4096, NULL, 6, NULL);
}
