#ifndef __MY_SNTP_H__
#define __MY_SNTP_H__

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "esp_sntp.h"

#include "mqtt_tcp.h"

extern time_t now;
void sntp_sync_time(struct timeval *tv);
void time_sync_notification_cb(struct timeval *tv);

void get_nettime_init(void);
void read_time(void);
#endif