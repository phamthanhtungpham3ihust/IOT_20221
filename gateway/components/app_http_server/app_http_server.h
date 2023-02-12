#ifndef __APP_HTTP_SERVER_H
#define __APP_HTTP_SERVER_H
#include <stdio.h>
#include "esp_http_server.h"

typedef void (*http_app_get_handle)(httpd_req_t *req);
typedef void (*http_app_post_handle)(char *buf, int len);

void start_webserver(void);
void stop_webserver(void);
void http_get_dhtt11_set_callback(void *cb);
void http_switch1_set_callback(void *cb);
void http_switch2_set_callback(void *cb);

#endif