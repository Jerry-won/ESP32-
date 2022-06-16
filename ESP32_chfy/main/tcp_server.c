/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// #include "tcp_server.h"
#include "main.h"

static char ESP32_Info_Buff[400];
char rx_buffer[700];
char monitor_epm12m_temp[700];
char Monitor_EPM12M_Data_Buf[1300];
// extern char device_topic[127];
bool is_complete = 0;
int data_lenth;
#define PORT    8080
extern C2R_mesh C2R_data;
extern Whether_Online isOnline;
static const char *TAG = "TCP server";

void HX_Mindray_Monitor_EPM12M_TX_Task(void *arg)
{
    int tcp_sock=-1; 
    int cout = 0;
    int i = 0;
    int j = 0;
    char flag = 0;
    while (1)
    {
        while (tcp_sock == -1)
        {
        /*初始化TCP server*/
        ESP_LOGE(__FILE__, "tcp server ... sock1= %d", tcp_sock);
        tcp_sock = tcp_server_init(8080);
        vTaskDelay(1000 / portTICK_PERIOD_MS); //延时1s
        ESP_LOGE(__FILE__, "tcp server ... sock2= %d", tcp_sock);
        }

        while(1)
        {
            is_complete = 0; //数据 未结束
            flag = 0;
            j = 0;
            data_lenth = recv(tcp_sock, rx_buffer, sizeof(rx_buffer)-1, 0);
            LED2_Flicker();
            if (data_lenth < 0) 
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            } 
            else if (data_lenth == 0) 
            {
                ESP_LOGW(TAG, "Connection closed");
            }
            else 
            {
                isOnline=ON_LINE;
                rx_buffer[data_lenth] = 0; // Null-terminate whatever is received and treat it like a string
                ESP_LOGI(TAG, "Received %d bytes: %s", data_lenth, rx_buffer);
                send(tcp_sock, "helloworld", strlen("helloworld"), 0);
                if(rx_buffer[0] == 0x0b)
                {
                        cout++;
                        ESP_LOGE(TAG, "cout: %d", cout);
                        rx_buffer[0] = '*';
                        flag = 1;
                }
                for (i = flag; i < data_lenth; i++)
                    {

                        if (rx_buffer[i] == '\\')
                        {
                            rx_buffer[i] = '#';
                        }
                        else if (rx_buffer[i] == 0x0d || rx_buffer[i] == 0x0a)
                        {
                            rx_buffer[i] = '*';
                        }

                        if (rx_buffer[i] == 0x1c && rx_buffer[i + 1] == 0x0d) //结束标志位 1c 0d
                        {

                            is_complete = 1;
                            break;
                        }

                        monitor_epm12m_temp[j++] = rx_buffer[i];
                    }

                    ESP_LOGW(TAG, " %d", data_lenth);
                    monitor_epm12m_temp[data_lenth] = '\0';
                    
                
                Foramt_Monitor_EPM10_Data_Deserialization(Monitor_EPM12M_Data_Buf, monitor_epm12m_temp);
                data_lenth = strlen(Monitor_EPM12M_Data_Buf);
                Monitor_EPM12M_Data_Buf[data_lenth] = '\0';
                // wifi_mesh_send_data(Monitor_EPM12M_Data_Buf,data_lenth,device_topic);
                ESP_LOGI(TAG, "Monitor_EPM12M_Data_Buf: %s",Monitor_EPM12M_Data_Buf);
                strcpy(C2R_data.device_child_data, Monitor_EPM12M_Data_Buf);
                strcpy(C2R_data.device_child_topic, device_topic);
                C2R_data.device_datalen=data_lenth;
            }
        } 
        shutdown(tcp_sock, 0);
        close(tcp_sock);
        tcp_sock = -1;
        ESP_LOGE(TAG, "tcp断开");
        isOnline = NOT_LINE; //device Online
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
   
}

int tcp_server_init(int port)
{
    ESP_LOGI(TAG, "等待设备连接......");
    char addr_str[128];
    // int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(port);
    ip_protocol = IPPROTO_IP;
 

    int listen_sock = socket(AF_INET, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) 
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return -1;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", AF_INET);
        return -1;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", port);

    err = listen(listen_sock, 1);
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        return -1;
    }

    while (1) 
    {
        ESP_LOGI(TAG, "Socket listening");
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);       
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        
        if (sock < 0) 
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
             return -1;
        }
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) 
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        // do_retransmit(sock);
        // shutdown(sock, 0);
        // close(sock);
         return sock;
    }

     return -1;
}

// void tcp_server_init(void)
// {
//     xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
// }
void Foramt_Monitor_EPM10_Data_Deserialization(char *Monitor_EPM12M_Data_Buff, char *temp_data)
{
    /* ESP32 Info*/
    Get_Format_JSON_ESP32_Info(ESP32_Info_Buff);
    sprintf(Monitor_EPM12M_Data_Buff, Format_JSON_ESP32_AND_Mindray_Monitor_RDY, ESP32_Info_Buff, temp_data);
}