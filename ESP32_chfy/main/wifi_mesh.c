/* Mesh Internal Communication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_mesh_internal.h"
#include "mesh_light.h"
#include "nvs_flash.h"
#include "mqtt_tcp.h"
#include "my_sntp.h"
#include "cJSON.h"
#include "main.h"
/*******************************************************
 *                Macros
 *******************************************************/

/*******************************************************
 *                Constants
 *******************************************************/
#define RX_SIZE          (1500)
#define TX_SIZE          (1460)

/*******************************************************
 *                Variable Definitions
 *******************************************************/
static const char *MESH_TAG = "mesh_main";
static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_running = true;
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;
static uint8_t mac[6];
#define Format_JSON_MESH_TABLE_SINGLE "{\"mac\":\""MACSTR"\",\"bit\":%d}"
#define Format_JSON_MESH_TABLE  "{\"root\":\""MACSTR"\",\"maclist\":[%s],\"maccout\":%d}"

char child_mac_str[18];
char root_mac_str[18];
char *child_sendData;
char now_char[200];
char uart_data_char[500];
uint8_t *uart_chid_data;
#define WIFI_NAME_ROOT "rdy_ms_601"
// #define WIFI_NAME_ROOT "rdy-ch501"
// #define WIFI_NAME_ROOT "realme_GT"
mesh_light_ctl_t light_on = {
    .cmd = MESH_CONTROL_CMD,
    .on = 1,
    .token_id = MESH_TOKEN_ID,
    .token_value = MESH_TOKEN_VALUE,
};

mesh_light_ctl_t light_off = {
    .cmd = MESH_CONTROL_CMD,
    .on = 0,
    .token_id = MESH_TOKEN_ID,
    .token_value = MESH_TOKEN_VALUE,
};

R2C_mesh R2C_data={
     .kid_mac={0,},
     .father_mac={0,},
     .sntp_time=0,
 };

C2R_mesh C2R_data={
    .device_child_data={0,},
    .device_child_topic={0,},
    .device_datalen=0,
    .data_count=0,
};

C2R_mesh *C2R_whatch;

/*******************************************************
 *                Function Declarations
 *******************************************************/

/*******************************************************
 *                Function Definitions
 *******************************************************/
char wifi_mesh_route_table_buff[1024];
char wifi_mesh_route_table_temp[600];
void send_wifi_mesh_table()
{

    int mesh_buff_len = 0;
    mesh_buff_len = strlen(wifi_mesh_route_table_buff);
    if (mesh_buff_len > 0)

    {       

        wifi_mesh_route_table_buff[mesh_buff_len] = '\0';

       // mqtt_send_message(Format_MESH_TABLE_TOPIC, wifi_mesh_route_table_buff, mesh_buff_len, 0);
    }
    memset(wifi_mesh_route_table_buff, 0, sizeof(wifi_mesh_route_table_buff));
    memset(wifi_mesh_route_table_temp, 0, sizeof(wifi_mesh_route_table_temp));
}

void wifi_mesh_table_task(void *arg)
{
    mesh_addr_t route_table[50];
    int route_table_size = 0;
    char route_table_buff[128];
    int mesh_temp_len = 0;
    int i = 0;
    while (1)
    {

        mesh_temp_len = 0;
        i = 0;
        /* 获取路由表   */
        esp_mesh_get_routing_table((mesh_addr_t *)&route_table,
                                   50 * 6, &route_table_size);
        /* 减掉自身 */
        route_table_size -= 1;
       // USER_LOGW(__FILE__, " mac_cout : %d", route_table_size);
        for (i = 0; i <= route_table_size; i++)
        {
            if (i > 0)
            {

                 sprintf(route_table_buff,Format_JSON_MESH_TABLE_SINGLE, MAC2STR(route_table[i].addr), i);
                //  ESP_LOGI(MESH_TAG, " mac: %s ", route_table_buff);
                  strcat(route_table_buff, wifi_mesh_route_table_temp);
                  strcat(wifi_mesh_route_table_temp, route_table_buff);

                //   wifi_mesh_send_data();
            }
            if ((i > 0) && (i % 10 == 0)) //到达10就一次
            {
                mesh_temp_len = strlen(wifi_mesh_route_table_temp);
                if (mesh_temp_len > 0)
                {
                    wifi_mesh_route_table_temp[mesh_temp_len - 1] = '\0';
                    sprintf(wifi_mesh_route_table_buff, Format_JSON_MESH_TABLE,MAC2STR(mac), wifi_mesh_route_table_temp, route_table_size);
                    send_wifi_mesh_table();
                }
            }
        }
        if ((i > 0) && (i % 10 != 0))//不到10就发一次 ， 比如有 16个  上面到达10个发了一次   还剩6个 上面不会执行 就由这个来发送
        {
            mesh_temp_len = strlen(wifi_mesh_route_table_temp);
            if (mesh_temp_len > 0)
            {
                wifi_mesh_route_table_temp[mesh_temp_len - 1] = '\0';
                sprintf(wifi_mesh_route_table_buff, Format_JSON_MESH_TABLE,MAC2STR(mac), wifi_mesh_route_table_temp, route_table_size);
                send_wifi_mesh_table();
            }
        }
//
        vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
    }
}

void esp_mesh_p2p_tx_main(void *arg)//根节点向子节点发送
{

    int i;
    esp_err_t err;
    int send_count = 0;
    mesh_addr_t route_table[50];
    int route_table_size = 0;
    mesh_data_t data;
    data.data = tx_buf;
    data.size = sizeof(tx_buf);
    data.proto = MESH_PROTO_BIN;
    data.tos = MESH_TOS_P2P;
    is_running = true;
    R2C_mesh *R2C_whatch;
        while (is_running) 
        {
           read_time();//获取网络时间
           R2C_data.sntp_time=now;
           ESP_LOGI(MESH_TAG, "layer:%d, rtableSize:%d, %s", mesh_layer,
                        esp_mesh_get_routing_table_size(),
                        (is_mesh_connected && esp_mesh_is_root()) ? "ROOT" : is_mesh_connected ? "NODE" : "DISCONNECT");
            /* non-root do nothing but print */
            if (!esp_mesh_is_root()) 
            {
                ESP_LOGI(MESH_TAG, "layer:%d, rtableSize:%d, %s", mesh_layer,
                        esp_mesh_get_routing_table_size(),
                        (is_mesh_connected && esp_mesh_is_root()) ? "ROOT" : is_mesh_connected ? "NODE" : "DISCONNECT");
                vTaskDelay(10 * 1000 / portTICK_RATE_MS);
                continue;
            }
            esp_mesh_get_routing_table((mesh_addr_t *) &route_table,
                                    50 * 6, &route_table_size);

            if (send_count && !(send_count % 100)) 
            {
                ESP_LOGI(MESH_TAG, "size:%d/%d,send_count:%d", route_table_size,
                        esp_mesh_get_routing_table_size(), send_count);
            }
            send_count++;
            tx_buf[25] = (send_count >> 24) & 0xff;
            tx_buf[24] = (send_count >> 16) & 0xff;
            tx_buf[23] = (send_count >> 8) & 0xff;
            tx_buf[22] = (send_count >> 0) & 0xff;

            if (send_count % 2) 
            {
                //memcpy(tx_buf, (uint8_t *)&light_on, sizeof(light_on));
                memcpy(tx_buf, (uint8_t *)&R2C_data, sizeof(R2C_data));
            } 
            else 
            {
                memcpy(tx_buf, (uint8_t *)&R2C_data, sizeof(R2C_data));
                // memcpy(tx_buf, (uint8_t *)&light_off, sizeof(light_off));
            }

            for (i = 1; i < route_table_size; i++) //为保证根节点不收到自身数据，路由表从1开始广播
            {
            err = esp_mesh_send(&route_table[i], &data, MESH_DATA_P2P, NULL, 0);
            R2C_whatch=(R2C_mesh*)data.data;
            // ESP_LOGI(MESH_TAG, "R2C_whatch->kid_mac:%s,R2C_whatch->father_mac:%s,R2C_whatch->sntp_time:%ld", R2C_whatch->kid_mac,R2C_whatch->father_mac,R2C_whatch->sntp_time);
                if (err) 
                {
                    ESP_LOGE(MESH_TAG,
                            "[ROOT-2-UNICAST:%d][L:%d]parent:"MACSTR" to "MACSTR", heap:%d[err:0x%x, proto:%d, tos:%d]",
                            send_count, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                            MAC2STR(route_table[i].addr), esp_get_minimum_free_heap_size(),
                            err, data.proto, data.tos);
                } 
                else if (!(send_count % 100)) 
                {
                            ESP_LOGW(MESH_TAG,
                            "[ROOT-2-UNICAST:%d][L:%d][rtableSize:%d]parent:"MACSTR" to "MACSTR", heap:%d[err:0x%x, proto:%d, tos:%d]",
                            send_count, mesh_layer,
                            esp_mesh_get_routing_table_size(),
                            MAC2STR(mesh_parent_addr.addr),
                            MAC2STR(route_table[i].addr), esp_get_minimum_free_heap_size(),
                            err, data.proto, data.tos);
                }
            }
            /* if route_table_size is less than 10, add delay to avoid watchdog in this task. */
            if (route_table_size < 10) 
            {
                vTaskDelay(1 * 1000 / portTICK_RATE_MS);
            }
        }    
    vTaskDelete(NULL);
}

static int err = 0;
static int send_count = 0;

void wifi_mesh_send_data(char *tx_data, int len, char *topic)
{
    ESP_LOGI(MESH_TAG, "data: %s", tx_data);
    if (!esp_mesh_is_root())
    {

        mesh_data_t data;
        data.data = tx_buf;
        data.size = sizeof(tx_buf);
        data.proto = MESH_PROTO_BIN;
        data.tos = MESH_TOS_P2P;
        
        send_count++;

        strcpy(C2R_data.device_child_data, tx_data);//把数据放入结构体
        strcpy(C2R_data.device_child_topic, topic);//把主题放入结构体
        C2R_data.device_datalen=len;
        C2R_data.data_count=send_count;

        memcpy(tx_buf, (uint8_t *)&C2R_data, sizeof(C2R_data));

        err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);//&route_table[i]
        C2R_whatch=(C2R_mesh*)data.data;
        ESP_LOGI(MESH_TAG, "C2R_whatch->device_child_topic:%s,C2R_whatch->device_datalen:%d,C2R_whatch->data_count:%d", C2R_whatch->device_child_topic,C2R_whatch->device_datalen,C2R_whatch->data_count);
                        
        if (err != ESP_OK)
        {
           ESP_LOGE(MESH_TAG,"data send failed");
        }
        else
        {
           ESP_LOGI(MESH_TAG,"data send succeed");
        }
    }
}

void esp_mesh_p2p_child_tx(void *arg)//子节点向根节点发送
{

    
    esp_err_t err;
    int send_count = 0;
    mesh_addr_t route_table[50];
    int route_table_size = 0;
    mesh_data_t data;
    data.data = tx_buf;
    data.size = sizeof(tx_buf);
    data.proto = MESH_PROTO_BIN;
    data.tos = MESH_TOS_P2P;
    is_running = true;
    C2R_mesh *C2R_whatch;
    while (is_running) 
    {

            ESP_LOGE(MESH_TAG, "layer:%d, rtableSize:%d, %s", mesh_layer,
                        esp_mesh_get_routing_table_size(),
                        (is_mesh_connected && esp_mesh_is_root()) ? "ROOT" : is_mesh_connected ? "NODE" : "DISCONNECT");
            /* non-root do nothing but print */
            if (!esp_mesh_is_root()) 
            {
                esp_mesh_get_routing_table((mesh_addr_t *) &route_table,
                                    50 * 6, &route_table_size);

                if (send_count && !(send_count % 100)) 
                {
                    ESP_LOGI(MESH_TAG, "size:%d/%d,send_count:%d", route_table_size,
                            esp_mesh_get_routing_table_size(), send_count);
                }
                send_count++;
                tx_buf[25] = (send_count >> 24) & 0xff;
                tx_buf[24] = (send_count >> 16) & 0xff;
                tx_buf[23] = (send_count >> 8) & 0xff;
                tx_buf[22] = (send_count >> 0) & 0xff;

                C2R_data.data_count=send_count;

                if (send_count % 2) 
                {
                   
                    memcpy(tx_buf, (uint8_t *)&C2R_data, sizeof(C2R_data));
                } 
                else 
                {
                    memcpy(tx_buf, (uint8_t *)&C2R_data, sizeof(C2R_data));
                   
                }

                    err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);//&route_table[i]
                    C2R_whatch=(C2R_mesh*)data.data;
                    // ESP_LOGI(MESH_TAG, "C2R_whatch->device_child_topic:%s,C2R_whatch->device_datalen:%d,C2R_whatch->data_count:%d", C2R_whatch->device_child_topic,C2R_whatch->device_datalen,C2R_whatch->data_count);
                        
                    if (err != ESP_OK) 
                    {
                        ESP_LOGE(MESH_TAG,"data send failed");
                    } 
                    else  
                    {
                        ESP_LOGI(MESH_TAG,"data send succeed");
                    }
                
                /* if route_table_size is less than 10, add delay to avoid watchdog in this task. */
                if (route_table_size < 10) 
                {
                    vTaskDelay(1 * 1000 / portTICK_RATE_MS);
                }

            }
            
    }    
    vTaskDelete(NULL);
}

void esp_mesh_p2p_root_rx(void *arg)//根节点接收子节点数据
{
    int recv_count = 0;
    esp_err_t err;
    mesh_addr_t from;
    int send_count = 0;
    mesh_data_t data;
    int flag = 0;
    data.data = rx_buf;
    data.size = RX_SIZE;
    is_running = true;
    C2R_mesh *C2R_child_whatch;
    while (is_running) 
    {
        data.size = RX_SIZE;
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        read_time();//获取网络时间
        if (err != ESP_OK || !data.size) 
        {
            ESP_LOGE(MESH_TAG, "err:0x%x, size:%d", err, data.size);
            continue;
        }
        // printf("data.size = %d\r\n",data.size);
        C2R_child_whatch=(C2R_mesh *)data.data;
        // ESP_LOGE(MESH_TAG, "C2R_child_whatch->device_child_data:%s,C2R_child_whatch->device_child_topic:%s,C2R_child_whatch->device_datalen:%d", C2R_child_whatch->device_child_data,C2R_child_whatch->device_child_topic,C2R_child_whatch->device_datalen);
        // mqtt_send_message(C2R_child_whatch->device_child_topic,C2R_child_whatch->device_child_data,C2R_child_whatch->device_datalen,0);
        mqtt_app_publish(C2R_child_whatch->device_child_topic,C2R_child_whatch->device_child_data);//发布mqtt
        recv_count++;
        if (!(recv_count % 1)) 
        {
            // ESP_LOGW(MESH_TAG,
            //          "[#RX:%d/%d][L:%d] parent:"MACSTR", receive from "MACSTR", size:%d, heap:%d, flag:%d[err:0x%x, proto:%d, tos:%d]",
            //          recv_count, send_count, mesh_layer,
            //          MAC2STR(mesh_parent_addr.addr), MAC2STR(from.addr),
            //          data.size, esp_get_minimum_free_heap_size(), flag, err, data.proto,
            //          data.tos);
        }
       vTaskDelay(1 * 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void esp_mesh_p2p_child_rx(void *arg)//子节点接收根节点数据
{
    int recv_count = 0;
    esp_err_t err;
    mesh_addr_t from;
    int send_count = 0;
    mesh_data_t data;
    int flag = 0;
    data.data = rx_buf;
    data.size = RX_SIZE;
    is_running = true;
    R2C_mesh *R2C_whatch_child;
    while (is_running) 
    {
        data.size = RX_SIZE;
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        R2C_whatch_child=(R2C_mesh *)data.data;
        // ESP_LOGI(MESH_TAG, "R2C_whatch_child->kid_mac:%s,R2C_whatch_child->father_mac:%s,R2C_whatch_child->sntp_time:%ld", R2C_whatch_child->kid_mac,R2C_whatch_child->father_mac,R2C_whatch_child->sntp_time);
        
        timesTamp_Box=R2C_whatch_child->sntp_time;//子节点获取网络时间
        if (err != ESP_OK || !data.size) 
        {
            ESP_LOGE(MESH_TAG, "err:0x%x, size:%d", err, data.size);
            continue;
        }
        // printf("data.size = %d\r\n",data.size);
        recv_count++;
        if (!(recv_count % 1)) 
        {
            // ESP_LOGW(MESH_TAG,
            //          "[#RX:%d/%d][L:%d] parent:"MACSTR", receive from "MACSTR", size:%d, heap:%d, flag:%d[err:0x%x, proto:%d, tos:%d]",
            //          recv_count, send_count, mesh_layer,
            //          MAC2STR(mesh_parent_addr.addr), MAC2STR(from.addr),
            //          data.size, esp_get_minimum_free_heap_size(), flag, err, data.proto,
            //          data.tos);
        }
       vTaskDelay(1 * 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

esp_err_t esp_mesh_comm_p2p_start(void)
{
    static bool is_comm_p2p_started = false;
    if (!is_comm_p2p_started) 
    {
        is_comm_p2p_started = true;
        
        if(esp_device_type_t == ESP32_ROOT_NODE)
        {
            xTaskCreate(esp_mesh_p2p_tx_main, "MPTX", 3072, NULL, 4, NULL);//根节点发送数据至子节点
            xTaskCreate(esp_mesh_p2p_root_rx, "MPRX", 3072, NULL, 7, NULL);//wifi mesh根节点接收数据任务
            xTaskCreate(wifi_mesh_table_task, "wifi_mesh_table_task", 3072, NULL, 3, NULL);//获取路由表任务
            // xTaskCreate(mqtt_send_task, "mqtt_send_task", 3072, NULL, 4, NULL);//mqtt发送任务
        }
        // else if(esp_device_type==HX_MINDRAY_MONITOR_EPM12M)
        else
        {
                xTaskCreate(esp_mesh_p2p_child_rx, "Mindray_node_rx", 3072, NULL, 5, NULL);//子节点接收根节点数据
            if(esp_device_type_t == HX_MINDRAY_MONITOR_EPM12M)
            {
                xTaskCreate(esp_mesh_p2p_child_tx, "Mindray_node_tx", 10000, NULL, 6, NULL);//子节点发送数据到根节点
            }
            
        }
    }
    return ESP_OK;
}

void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    mesh_addr_t id = {0,};
    static uint16_t last_layer = 0;

    switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_CHILD_CONNECTED: {
        mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<连接到子节点>aid:%d, "MACSTR"",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));
        snprintf(child_mac_str, sizeof(child_mac_str), "%02x%02x-%02x%02x-%02x%02x",child_connected->mac[0],
                    child_connected->mac[1],child_connected->mac[2],child_connected->mac[3],child_connected->mac[4],child_connected->mac[5]);
        strcpy(R2C_data.kid_mac, child_mac_str);
        
        ESP_LOGI(MESH_TAG,"child_mac:%s",child_mac_str);
    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<子节点断开>aid:%d, "MACSTR"",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<通过添加新加入的子节点更改路由表>add %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<通过移除子节点更改路由表>remove %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_NO_PARENT_FOUND: {
        mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
        ESP_LOGI(MESH_TAG, "<未发现父节点>scan times:%d",
                 no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<连接到父节点,层数改变>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR", duty:%d",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr), connected->duty);
        last_layer = mesh_layer;
        // mesh_connected_indicator(mesh_layer);
        is_mesh_connected = true;
        LED2_ON();
        if (esp_mesh_is_root()) 
        {
            esp_netif_dhcpc_start(netif_sta);  
        }
        esp_mesh_comm_p2p_start();

    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
        mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<父节点断开>reason:%d",
                 disconnected->reason);
        is_mesh_connected = false;
        // mesh_disconnected_indicator();
        mesh_layer = esp_mesh_get_layer();
        LED2_OFF();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE: {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        ESP_LOGI(MESH_TAG, "<层数改变>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
        // mesh_connected_indicator(mesh_layer);
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS: {
        mesh_event_root_address_t *root_addr = (mesh_event_root_address_t *)event_data;
        ESP_LOGI(MESH_TAG, "<根节点地址>root address:"MACSTR"",
                 MAC2STR(root_addr->addr));

        snprintf(root_mac_str, sizeof(root_mac_str), "%02x%02x-%02x%02x-%02x%02x",root_addr->addr[0],
                    root_addr->addr[1],root_addr->addr[2],root_addr->addr[3],root_addr->addr[4],root_addr->addr[5]);
        strcpy(R2C_data.father_mac, root_mac_str);
        
        ESP_LOGI(MESH_TAG,"root_mac:%s",root_mac_str);
        
    }
    break;
    case MESH_EVENT_VOTE_STARTED: {
        mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<投票开始>attempts:%d, reason:%d, rc_addr:"MACSTR"",
                 vote_started->attempts,
                 vote_started->reason,
                 MAC2STR(vote_started->rc_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STOPPED: {
        ESP_LOGI(MESH_TAG, "<投票结束>");
        break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
        mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<从新选举的根节点候选者发送根节点切换请求>reason:%d, rc_addr:"MACSTR"",
                 switch_req->reason,
                 MAC2STR( switch_req->rc_addr.addr));
    }
    break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
        /* new root */
        mesh_layer = esp_mesh_get_layer();
        esp_mesh_get_parent_bssid(&mesh_parent_addr);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:"MACSTR"", mesh_layer, MAC2STR(mesh_parent_addr.addr));
    }
    break;
    case MESH_EVENT_TODS_STATE: {
        mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    }
    break;
    case MESH_EVENT_ROOT_FIXED: {
        mesh_event_root_fixed_t *root_fixed = (mesh_event_root_fixed_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
                 root_fixed->is_fixed ? "fixed" : "not fixed");
    }
    break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
        mesh_event_root_conflict_t *root_conflict = (mesh_event_root_conflict_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_ASKED_YIELD>"MACSTR", rssi:%d, capacity:%d",
                 MAC2STR(root_conflict->addr),
                 root_conflict->rssi,
                 root_conflict->capacity);
    }
    break;
    case MESH_EVENT_CHANNEL_SWITCH: {
        mesh_event_channel_switch_t *channel_switch = (mesh_event_channel_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", channel_switch->channel);
    }
    break;
    case MESH_EVENT_SCAN_DONE: {
        mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d",
                 scan_done->number);
    }
    break;
    case MESH_EVENT_NETWORK_STATE: {
        mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
                 network_state->is_rootless);
    }
    break;
    case MESH_EVENT_STOP_RECONNECTION: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    }
    break;
    case MESH_EVENT_FIND_NETWORK: {
        mesh_event_find_network_t *find_network = (mesh_event_find_network_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:"MACSTR"",
                 find_network->channel, MAC2STR(find_network->router_bssid));
    }
    break;
    case MESH_EVENT_ROUTER_SWITCH: {
        mesh_event_router_switch_t *router_switch = (mesh_event_router_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, "MACSTR"",
                 router_switch->ssid, router_switch->channel, MAC2STR(router_switch->bssid));
    }
    break;
    case MESH_EVENT_PS_PARENT_DUTY: {
        mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_PS_PARENT_DUTY>duty:%d", ps_duty->duty);
    }
    break;
    case MESH_EVENT_PS_CHILD_DUTY: {
        mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_PS_CHILD_DUTY>cidx:%d, "MACSTR", duty:%d", ps_duty->child_connected.aid-1,
                MAC2STR(ps_duty->child_connected.mac), ps_duty->duty);
    }
    break;
    default:
        ESP_LOGI(MESH_TAG, "unknown id:%d", event_id);
        break;
    }
}

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{

    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR, IP2STR(&event->ip_info.ip));

    if(esp_device_type_t == ESP32_ROOT_NODE)
    {
        get_nettime_init();//sntp初始化
        mqtt_app_start();//mqtt配置及初始化
    }

}

void wifi_mesh_init(void)
{
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL));
    /*  wifi initialization */
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_start());
    /*  mesh initialization */
    ESP_ERROR_CHECK(esp_mesh_init());
    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));
    /*  set mesh topology */
    ESP_ERROR_CHECK(esp_mesh_set_topology(0));
    /*  set mesh max layer according to the topology */
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(6));
    ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(1));
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(128));
    if(esp_device_type_t == ESP32_ROOT_NODE)
    {
        esp_mesh_allow_root_conflicts(true);              //设置是否允许一个网络中存在多个根
        esp_mesh_set_self_organized(true, true);          //启用自组网
    }
#ifdef CONFIG_MESH_ENABLE_PS
    /* Enable mesh PS function */
    ESP_ERROR_CHECK(esp_mesh_enable_ps());
    /* better to increase the associate expired time, if a small duty cycle is set. */
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(60));
    /* better to increase the announce interval to avoid too much management traffic, if a small duty cycle is set. */
    ESP_ERROR_CHECK(esp_mesh_set_announce_interval(600, 3300));
#else
    /* Disable mesh PS function */
    ESP_ERROR_CHECK(esp_mesh_disable_ps());
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(10));
#endif
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    /* mesh ID */
    memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);
    /* router */
    cfg.channel = 0;
    cfg.router.ssid_len = strlen(WIFI_NAME_ROOT);
    memcpy((uint8_t *) &cfg.router.ssid, WIFI_NAME_ROOT, cfg.router.ssid_len);
    if(esp_device_type_t == ESP32_ROOT_NODE)
    {
        
        memcpy((uint8_t *) &cfg.router.password, "Admin@123",
            strlen("Admin@123"));
    }
    else
    {
        memcpy((uint8_t *) &cfg.router.password, "12345623",
           strlen("12345623"));
    }

    /* mesh softAP */
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(3));
    cfg.mesh_ap.max_connection = 10;
    cfg.mesh_ap.nonmesh_max_connection = 0;
    memcpy((uint8_t *) &cfg.mesh_ap.password, "MAP_PASSWD",
           strlen("MAP_PASSWD"));
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
    /* mesh start */
    ESP_ERROR_CHECK(esp_mesh_start());//mesh开始运行

    if(esp_device_type_t == ESP32_ROOT_NODE)
    {
        wifi_config_t wifi_config = {
                .sta = {
                    // .ssid = "rdy_ms_601",            //wifi名称
                    .ssid = WIFI_NAME_ROOT,            //wifi名称
                    .password = "Admin@123",        //wifi密码
                    // .password = "123456789",        //wifi密码
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK, //加密方式
                    .pmf_cfg = {
                                .capable = false,
                                .required = false},
                },
        };
        esp_mesh_set_parent(&wifi_config, NULL, MESH_ROOT, MESH_ROOT_LAYER);
    }
    

#ifdef CONFIG_MESH_ENABLE_PS
    /* set the device active duty cycle. (default:10, MESH_PS_DEVICE_DUTY_REQUEST) */
    ESP_ERROR_CHECK(esp_mesh_set_active_duty_cycle(CONFIG_MESH_PS_DEV_DUTY, CONFIG_MESH_PS_DEV_DUTY_TYPE));
    /* set the network active duty cycle. (default:10, -1, MESH_PS_NETWORK_DUTY_APPLIED_ENTIRE) */
    ESP_ERROR_CHECK(esp_mesh_set_network_duty_cycle(CONFIG_MESH_PS_NWK_DUTY, CONFIG_MESH_PS_NWK_DUTY_DURATION, CONFIG_MESH_PS_NWK_DUTY_RULE));
#endif
    ESP_LOGI(MESH_TAG, "mesh starts successfully, heap:%d, %s<%d>%s, ps:%d\n",  esp_get_minimum_free_heap_size(),
             esp_mesh_is_root_fixed() ? "root fixed" : "root not fixed",
             esp_mesh_get_topology(), esp_mesh_get_topology() ? "(chain)":"(tree)", esp_mesh_is_ps_enabled());
}