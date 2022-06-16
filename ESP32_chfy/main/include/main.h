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
#include "esp_log.h"
#include "esp_tls.h"
#include "ethernet.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "basic_imformation.h"
#include "uart_stephan.h"



typedef enum
{
   ESP32_ROOT_NODE = 5U,         //根节点  
   STEPHAN = 8U,                 //眉山史蒂芬呼吸机Sophie-conventional
   DAVID_YP_90AC = 9U,           //眉山戴维婴儿培养箱YP-90AC
   HX_MINDRAY_MONITOR_UMEC10=10U, //眉山迈瑞监护仪uMEC10
   HX_MINDRAY_MONITOR_EPM12M=11U, //眉山迈瑞监护仪EPM12M
   MINDRAY_NB350 = 12U,           //眉山迈瑞呼吸机NB350  
   SLE5000=16U,                  //成华迈瑞SLE5000呼吸机
   SMITH_JSB=17U,
   DAVID_YP_90AB = 18U,          //成华戴维婴儿培养箱YP-90AB
} Device_TaskType;

Device_TaskType esp_device_type_t;

typedef enum
{
   NESH_ROOT=1U,
   MINDRAY_HL7=2U,
   UART_DEVICE=3U,
}Device_Category;

Device_Category esp_device_category_t;
#endif
