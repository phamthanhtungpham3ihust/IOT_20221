#include "app_http_server.h"
#include <esp_http_server.h>
#include "esp_log.h"

static const char *TAG = "app_http_server";
static httpd_handle_t server = NULL;

extern const uint8_t web_start[] asm("_binary_web_html_start");
extern const uint8_t web_end[] asm("_binary_web_html_end");

static http_app_get_handle http_get_dht11_handle = NULL;
static http_app_post_handle http_switch1_handle = NULL;


/* An HTTP GET handler */
static esp_err_t html_web_handler(httpd_req_t *req)
{
  
    httpd_resp_send(req, (const char*)web_start, web_end-web_start);
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        // ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t html_web = {
    .uri       = "/index.html",
    .method    = HTTP_GET,
    .handler   = html_web_handler,
    .user_ctx  = NULL
};

static httpd_req_t *get_req;
void http_send_response(char *data, int len)
{
    httpd_resp_send(get_req, (const char*)data, len); 
}

/* An HTTP GET handler */
static esp_err_t dht_get_data_handler(httpd_req_t *req)
{
    http_get_dht11_handle(req);
    return ESP_OK;
}

static const httpd_uri_t dht11 = {
    .uri       = "/get_data_dht11",
    .method    = HTTP_GET,
    .handler   = dht_get_data_handler,
    .user_ctx  = NULL
};


static esp_err_t switch1_data_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    int len = httpd_req_recv(req, buf, 100);
    http_switch1_handle(buf, len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;    
}

static const httpd_uri_t sw1 = {
    .uri       = "/switch1",
    .method    = HTTP_POST,
    .handler   = switch1_data_handler,
    .user_ctx  = NULL
};

static esp_err_t slider_data_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    int len = httpd_req_recv(req, buf, 100);
    printf("%s\n", buf);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;     
}

static const httpd_uri_t slider = {
    .uri       = "/slider",
    .method    = HTTP_POST,
    .handler   = slider_data_handler,
    .user_ctx  = "slider"
};

void start_webserver(void)
{
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &html_web);
        httpd_register_uri_handler(server, &dht11);
        httpd_register_uri_handler(server, &sw1);
        httpd_register_uri_handler(server, &slider);
    }
    else{
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void)
{
    httpd_stop(server);
}

void http_get_dhtt11_set_callback(void *cb)
{
    if(cb){
        http_get_dht11_handle = cb;
    }
}

void http_switch1_set_callback(void *cb)
{
    if(cb){
        http_switch1_handle = cb;
    }
}

