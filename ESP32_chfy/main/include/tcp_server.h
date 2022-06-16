#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
void Foramt_Monitor_EPM10_Data_Deserialization(char *Monitor_EPM12M_Data_Buff, char *temp_data);
int tcp_server_init(int port);
void HX_Mindray_Monitor_EPM12M_TX_Task(void *arg);
extern bool is_complete;
extern int data_lenth;
extern char Monitor_EPM12M_Data_Buf[1300];
#endif