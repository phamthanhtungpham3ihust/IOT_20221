set(app_src app_mqtt.c)
set(pri_req nvs_flash mqtt)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

