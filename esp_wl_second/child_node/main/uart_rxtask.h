#ifndef __UART_RXTASK_H__
#define __UART_RXTASK_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
extern uint8_t* uart_rx_data;
void init(void); 
void uart_task(void);
int sendData(const char* logName, const char* data);

#endif