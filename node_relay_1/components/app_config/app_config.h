#ifndef __APP_CONFIG_H_
#define __APP_CONFIG_H_

#include <stdio.h>

typedef enum{
    PROV_NONE,
    PROV_SMART_CONFIG,
    PROV_ACCESS_POINT
}  provision_type_t;


void app_config(void);

#endif