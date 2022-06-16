#ifndef  __UART_STEPHAN_H__
#define  __UART_STEPHAN_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

int  get_mac_addrs(void);
void uart_start(void);

#endif