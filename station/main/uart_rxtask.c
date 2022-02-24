/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "main.h"
static const int RX_BUF_SIZE = 1024;
static void tx_task(void *arg);
static void rx_task(void *arg);
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)
char wl_rx_buf[]="hello";
uint8_t* uart_rx_data;
char data_buf[]= "MSH|^~\&|||||||ORU^R01|503|P|2.3.1|OBX||NM|171^Dia|2105|-100||||||F||APERIODIC|00000000000000OBX||NM|172^Mean|2105|-100||||||F||APERIODIC|00000000000000OBX||NM|170^Sys|2105|-100||||||F||APERIODIC|00000000000000OBX||NM|173^NIBP_PR|2105|-100||||||F||APERIODIC|00000000000000";
#define LED_PIN     (2)   //可通过宏定义，修改引脚
void init(void) 
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 192000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);//输出;
    gpio_set_level(LED_PIN, 0); //拉低
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) 
    {
        sendData(TX_TASK_TAG, data_buf);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

    while (1) 
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) 
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            uart_rx_data=data;
            // ESP_LOGI(RX_TASK_TAG, "uart_rx_data: %s", uart_rx_data); 
            // strcpy(wl_rx_buf,(char*)data);
            // // memcpy(wl_rx_buf,data,strlen(*data));
            // ESP_LOGI(RX_TASK_TAG, "wl_rx_buf: %s", wl_rx_buf);            
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            gpio_set_level(LED_PIN, 1);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
        gpio_set_level(LED_PIN, 0);
    }


    free(data);

}

void uart_rx_task(void)
{
     init();
     xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //  xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}

void uart_tx_task(void)
{
     init();
     xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //  xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}