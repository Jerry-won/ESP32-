/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// #include "tcp_client.h"
#include "main.h"
#define HOST_IP_ADDR "192.168.3.13"
#define Client_PORT 4601

static const char *TAG = "TCP client";
// static const char *payload = "Message from ESP32 ";
int tcp_cli_sock = -1;
int *sock_point=&tcp_cli_sock;
char Monitor_UMEC10_Data_Format_Buff[1024 * 2];
extern TaskHandle_t HX_Mindray_Monitor_UMEC10_TX_TaskHandle;
int tcp_client_init(void)
{
   
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    assert(Client_PORT);

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(4601);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    ESP_LOGI(TAG, "client_host_ip_addr %s", host_ip);
    ESP_LOGI(TAG, "client_host_port %d", Client_PORT);

    int tcp_client_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (tcp_client_sock < 0) 
    {
        ESP_LOGE(TAG, "套接字创建失败: errno %d", errno);
        return -1;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, 4601);
    
    struct timeval timeout = {5, 0};
    setsockopt(tcp_client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    
    int err = connect(tcp_client_sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
    if (err != 0) 
    {
        ESP_LOGE(TAG, "套接字链接失败: errno %d", errno);
        return -1;
    }
    ESP_LOGI(TAG, "Successfully connected");
    return tcp_client_sock;
}

void HX_Mindray_Monitor_UMEC10_TX_Task(void *client_sock)
{

     int len = 0;
    // char *sendHl7 ="MSH|^~\\&|||||||QRY^R02|1203|P|2.3.1\rQRD|20060731145557|R|I|Q895211|||||RES\rQRF|MON||||0&0^1^1^0^101&500&501&502\rQRF|MON||||0&0^1^1^0^160&161&220&222\rQRF|MON||||0&0^1^1^0^170&171&172&151\rQRF|MON||||0&0^1^1^0^503&504&505\rQRF|MON||||0&0^1^1^0^200&201&202&203\r1C0D";
    while (1)
    {
       if (tcp_cli_sock != -1)
        {

            /*1.Build request Data*/
            char sendHl7[300] = {0};
            len = 0;
            char hh1[] = {0x0B};
            char hh2[] = {0x1C,0x0D};
            // char hh3[] = {0x0D};

            len = sprintf(sendHl7, Mindray_Monitor_Format_JSON_IPM8_GET_DATA); //
            // printf("sendHl7:%s",sendHl7);
            /*2. send request data */
            // ESP_LOGI(TAG, "tcp_cli_sock:%d", tcp_cli_sock);
            send(tcp_cli_sock, hh1, 1, 0);       //ͷ
           int err= send(tcp_cli_sock, sendHl7, len, 0); //��������
            send(tcp_cli_sock, hh2, 2, 0);       //β
            // send(tcp_cli_sock, hh3, 1, 0);       //β
            memset(sendHl7, 0, sizeof(sendHl7));
            // printf("esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
            if(err<0)
            {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                esp_restart();
            }
            // else
            // {
            //     ESP_LOGE(TAG, "发送成功");
            // }
            
        }
            
           vTaskDelay(3000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


void HX_Mindray_Monitor_UMEC10_RX_Task(void *asg)
{

    long hl7Len = 0;
    int error_conntion_cout = 0;
    // int len = 0;
    // sprintf(device_topic, Format_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    while (1)
    {
        /*1.TCP Client Connect*/
        while (tcp_cli_sock <= -1)
        {
            *sock_point = tcp_client_init();//DEVICE_IP host_ip
            ESP_LOGW(TAG, "TCP Client connect....");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            error_conntion_cout++;
            if (error_conntion_cout >= 20)
            {
                esp_restart();
            }
            
        }
        error_conntion_cout = 0;
        ESP_LOGI(TAG, "TCP Client succeed.");

        while (1)
        {   
            memset(Monitor_UMEC10_RX_Buffer, 0, sizeof(Monitor_UMEC10_RX_Buffer));
            is_complete = 0; //数据 未结束

            hl7Len = recv(tcp_cli_sock, Monitor_UMEC10_RX_Buffer, sizeof(Monitor_UMEC10_RX_Buffer) - 1, 0);
            // ESP_LOGI(TAG,"recv_data:%s\r\n",Monitor_UMEC10_RX_Buffer);
            
            if (hl7Len <= 0)
            {
                ESP_LOGW(TAG, "recv failed: errno %d", errno); //����С����
                break;
            }
            else //������
            {
                // LED2_Flicker();
                isOnline = ON_LINE; //device Online
                Monitor_UMEC10_RX_Buffer[hl7Len] ='\0'; 
                Monitor_UMEC10_Data_Form_Send(Monitor_UMEC10_RX_Buffer,hl7Len);
            }
            
            vTaskDelay(100 / portTICK_PERIOD_MS);
      
        }
            if(tcp_cli_sock != -1)
            {
                shutdown(tcp_cli_sock, 0);
                close(tcp_cli_sock);
                tcp_cli_sock = -1;
                ESP_LOGE(TAG, "关闭socket并重新开始");
                // esp_restart();
                isOnline = NOT_LINE; //device Online
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }

    }
    vTaskDelete(NULL);
}

char dataBuffs[1024];
void Monitor_UMEC10_Data_Form_Send(char *data, int len)
{
    int i = 0;
    int j = 0;

    int flag = 0;
    // ESP_LOGW(TAG, "data len = %d", len);

    while (i < len)
    {

        if (data[i] == '\\')
        {
            data[i] = '#';
        }

        if (data[i] == 0x4d && data[i + 1] == 0x53 && data[i + 2] == 0x48)
        {
            flag++;
        }
        if (flag == 1)
        {
            // if (data[i] != 0x0b && data[i] != 0x20 && data[i] != 0x0d)
            // {
            //    dataBuffs[j++] = data[i];
            // }

            if (data[i] == 0x0d || data[i] == 0x0a)
            {
                dataBuffs[j++] = '*';
            }
            else
            {

                dataBuffs[j++] = data[i];
            }
        }
        else if (flag == 2)
        {
            dataBuffs[j - 3] = '\0';

            // // dataBuffs[j - ] = '*';

            Foramt_Monitor_EPM10_Data_Deserialization(Monitor_UMEC10_Data_Format_Buff, dataBuffs);

            int lens = strlen(Monitor_UMEC10_Data_Format_Buff);
            Monitor_UMEC10_Data_Format_Buff[lens] = '\0';
 
            wifi_mesh_send_data(Monitor_UMEC10_Data_Format_Buff, lens, device_topic);
            ESP_LOGI(TAG, "%s", dataBuffs);
            memset(Monitor_UMEC10_Data_Format_Buff, 0, sizeof(Monitor_UMEC10_Data_Format_Buff));
            memset(dataBuffs, 0, sizeof(dataBuffs));
            j = 0;
            flag = 0;
            i -= 1;
        }

        i++;
    }
}