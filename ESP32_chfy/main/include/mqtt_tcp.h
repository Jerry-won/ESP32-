#ifndef __MQTT_TCP_H__
#define __MQTT_TCP_H__

#include "main.h"

void mqtt_app_publish(char* topic, char *publish_string);
void mqtt_app_start(void);
void mqtt_send_message(const char *topic, char *data, int len, int qos);
#endif