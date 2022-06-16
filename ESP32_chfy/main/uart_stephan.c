#include "main.h"

#define RX_DATA 70 //接收到的字节数
#define TXD_PIN (GPIO_NUM_4)//PIN4
#define RXD_PIN (GPIO_NUM_5)//PIN5
// 眉山 STPHAN 呼吸机
// 开始字节[STX]：  0X02
// 请求命令[GET ?]: 0x47 0x45 0x54 0x3?
// ? = 1   GET1请求呼吸机的测量数据
// ? = 2   GET2请求警报数据，
// ? = 3   GET3请求基本设置
// 结束字节[ETX]：  0X03
char uart_tx_data1[6] = {0X02,0x47,0x45,0x54,0x31,0X03};//GET 1
char uart_tx_data0[6] = {0X02,0x47,0x45,0x54,0x32,0X03};//GET 2
// char uart_tx_data1[] = {0X02,0x47,0x45,0x54,0x33,0X03};//GET 3
char uart_tx_data2[13]={0X55,0X0A,0X01,0X01,0XCC,0XCC,0XCC,0XCC,0XCC,0X6B,0X4F,0XAA};//宁波daivd 婴儿床 读设备系统参数;
// char uart_tx_data3[5]={0X1B,0X20,0X53,0X8E,0X0D};//Mindray呼吸机--配置主从模式命令
char uart_tx_data3[5]={0X1B,0X25,0X40,0X0D};//Mindray呼吸机--获取设备参数
char uart_tx_data4[]={0X55,0XAA,0X05,0X06,0X01,0X4B,0X00,0XA9};//佳士比(莫得搞)

extern C2R_mesh C2R_data;
char Monitor_STEPHAN_Data_Format_Buff[500];
static const int RX_BUF_SIZE = 1024;

static void init(void) 
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 57600,//迈瑞呼吸机57600，史蒂芬呼吸机9600，婴儿床9600
        .data_bits = UART_DATA_8_BITS,//8bit数据位
        .parity = UART_PARITY_DISABLE,//奇偶校验位
        .stop_bits = UART_STOP_BITS_1,//1bit停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData_uart(const char* logName, const char* data)
{
    const int len = strlen(data);
    // ESP_LOGW(logName, " %d bytes", len);
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
    if(esp_device_type_t == STEPHAN)
    {     
        sendData_uart(TX_TASK_TAG,  uart_tx_data1);
        sendData_uart(TX_TASK_TAG,  uart_tx_data0);//请求史蒂芬呼吸机报警参数
    }
   else if(esp_device_type_t == MINDRAY_NB350)//迈瑞呼吸机
    {
        sendData_uart(TX_TASK_TAG,  uart_tx_data3);
    }
    
    else if(esp_device_type_t == DAVID_YP_90AC||esp_device_type_t == DAVID_YP_90AB)//婴儿培养箱
    {//记得配置暖箱，每种型号地址码不相同
        sendData_uart(TX_TASK_TAG,  uart_tx_data2); 
    }

    else if(esp_device_type_t == SMITH_JSB)//佳士比输注泵(老板说可以搞，但我觉得没法搞，要不你试试)
    {
        char send1=0x00,send2=0xA9;
        for(char i=0;i<sizeof(uart_tx_data4);i++)
        {
            // sendData_uart(TX_TASK_TAG,  uart_tx_data4+i);
          char txbytes= uart_write_bytes(UART_NUM_1,uart_tx_data4+i, 1);
           ESP_LOGI(TX_TASK_TAG, "Wrote %d bytes", txbytes);
        }
    }

        vTaskDelay(2000 / portTICK_PERIOD_MS);     
    }
}

static void rx_task(void *arg)
{   
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);//开辟堆区空间，尽量不要使用data这个变量，否则会出现意想不到的BUG哦
    while (1) 
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) 
        {
            uint8_t rx_data1[ RX_DATA + 1];
            char  hex_buffer[ RX_DATA + 1];
            data[rxBytes] = 0;//截取数组真实长度
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes", rxBytes);
            memset(rx_data1 ,0 ,RX_DATA + 1);
            memcpy(rx_data1 ,data ,rxBytes);
            ESP_LOG_BUFFER_HEX(RX_TASK_TAG,rx_data1,rxBytes);//十六进制打印           

        /*十六进制转字符串*/
        for (int i = 0; i < rxBytes; i ++) 
        {
            sprintf(hex_buffer + 2 * i, "%02x",rx_data1[i]);
        }
            // ESP_LOGW(RX_TASK_TAG , "data:%s", hex_buffer);
            C2R_data.device_datalen = rxBytes;
            //判断设备在线状态与数据传输状态
                if((rxBytes == 42) || (rxBytes == 27)||(rxBytes == 65))//收到的字节数为42(stephan)或27(david)
                {
                    isOnline = ON_LINE;//设备在线标志位
                    is_complete = 1;//传输完成标志位
                }
                else
                {
                    isOnline = NOT_LINE;
                    is_complete = 0;
                }
            Foramt_Monitor_EPM10_Data_Deserialization(Monitor_STEPHAN_Data_Format_Buff,hex_buffer);
            wifi_mesh_send_data(Monitor_STEPHAN_Data_Format_Buff, rxBytes, device_topic);
            /*标志位清零*/
            isOnline = NOT_LINE;
            is_complete = 0;
        }
    }
    free(data);//释放堆区空间
}

void uart_start(void)
{   
    init();  
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}