#ifndef __MAIN_H__
#define __MAIN_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include "dht11.h"

#include <sys/socket.h>
#include "mqtt_client.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/uart.h"
#include "driver/uart_select.h"
#include "mqtt_tcp.h"
#include "my_sntp.h"
#include "cJSON.h"
#include "wifi_mesh.h"
#include "mesh_light.h"
#include "uart_rxtask.h"

#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_tls.h"

typedef enum
{
   ESP32_ROOT_NODE = 0U,//根节点
   ESP32_CHILD_NODE = 1U,     //子节点
   ESP32_UART_TX_NODE = 2U, //UART发送
} Device_TaskType;

Device_TaskType esp_device_type;
#endif
