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
char *sendData;
char now_char[200];
char uart_data_char[500];
uint8_t *uart_chid_data;
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

// mesh_mac test_tx_rx={
//     .child_mac={0,},
//     .root_mac={0,},
// };
 mesh_mac test_tx_rx={
     .test_cmd=123,
     .test_value=156,
     .kid_mac={0,},
     .father_mac={0,},
 };

// mesh_child_to_root mesh_test={
//     .cmd=4,
//     .value=3,
//     .uart_data=2,
// };
mesh_child_to_root mesh_test;
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
    mesh_addr_t route_table[CONFIG_MESH_ROUTE_TABLE_SIZE];
    int route_table_size = 0;
    char route_table_buff[128];
    int mesh_temp_len = 0;
    int i = 0;

  
    while (1)
    {

        mesh_temp_len = 0;
        i = 0;
        /* ???????????????   */
        esp_mesh_get_routing_table((mesh_addr_t *)&route_table,
                                   CONFIG_MESH_ROUTE_TABLE_SIZE * 6, &route_table_size);
        /* ???????????? */
        route_table_size -= 1;
       // USER_LOGW(__FILE__, " mac_cout : %d", route_table_size);
        for (i = 0; i <= route_table_size; i++)
        {
            if (i > 0)
            {

                 sprintf(route_table_buff,Format_JSON_MESH_TABLE_SINGLE, MAC2STR(route_table[i].addr), i);
                 ESP_LOGI(MESH_TAG, " mac: %s ", route_table_buff);
                  strcat(route_table_buff, wifi_mesh_route_table_temp);
                  strcat(wifi_mesh_route_table_temp, route_table_buff);

                //   wifi_mesh_send_data();
            }
            if ((i > 0) && (i % 10 == 0)) //??????10?????????
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
        if ((i > 0) && (i % 10 != 0))//??????10???????????? ??? ????????? 16???  ????????????10???????????????   ??????6??? ?????????????????? ?????????????????????
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

void mqtt_send_task(void *arg)
{
    while(1)
    {
        cJSON *pRoot = cJSON_CreateObject();                         // ??????JSON???????????????
        cJSON *pValue = cJSON_CreateObject();                        // ??????JSON???????????????
        time(&now);
        sprintf(now_char,"%ld",now);
        //  sprintf(uart_data_char,"%s",uart_chid_data);

        cJSON_AddItemToObject(pRoot,"deviceInfo",pValue);    // ?????????????????????????????????????????????
        cJSON_AddStringToObject(pValue, "name","wl");
        cJSON_AddStringToObject(pValue,"timesTamp_Box",now_char);              // ?????????
        cJSON_AddStringToObject(pValue,"box_code",child_mac_str);              // ??????ESP32???mac??????
        cJSON_AddStringToObject(pValue,"root_mac",root_mac_str);              // ?????????mac??????
        cJSON_AddStringToObject(pRoot,"devicedata",(char*)uart_chid_data);
        sendData = cJSON_Print(pRoot);  

        ESP_LOGE(MESH_TAG, "sendData:%s", sendData);
        // esp_mqtt_client_publish(client, "esp/dht11", (char *)sendData, strlen(sendData), 0, 0);//????????????????????????????????????
        mqtt_app_publish("hl7/7c9e-bdf3-dabc/data",sendData);
        vTaskDelay(2 * 1000 / portTICK_PERIOD_MS);

    }


}

void esp_mesh_p2p_tx_main(void *arg)
{

        int i;
        esp_err_t err;
        int send_count = 0;
        mesh_addr_t route_table[CONFIG_MESH_ROUTE_TABLE_SIZE];
        int route_table_size = 0;
        mesh_data_t data;
        data.data = tx_buf;
        data.size = sizeof(tx_buf);
        data.proto = MESH_PROTO_BIN;
        data.tos = MESH_TOS_P2P;
        is_running = true;

        while (is_running) 
        {
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
                                    CONFIG_MESH_ROUTE_TABLE_SIZE * 6, &route_table_size);

            if (send_count && !(send_count % 100)) {
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
                memcpy(tx_buf, (uint8_t *)&test_tx_rx, sizeof(test_tx_rx));
            } 
            else 
            {
                memcpy(tx_buf, (uint8_t *)&test_tx_rx, sizeof(test_tx_rx));
                // memcpy(tx_buf, (uint8_t *)&light_off, sizeof(light_off));
            }

            for (i = 0; i < route_table_size; i++) 
            {
            err = esp_mesh_send(&route_table[i], &data, MESH_DATA_P2P, NULL, 0);

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

void esp_mesh_p2p_rx_main(void *arg)
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
    mesh_light_ctl_t *my_tx;
    mesh_mac *my_tx_test;
    mesh_child_to_root *child_tx;

    while (is_running) 
    {
        data.size = RX_SIZE;
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        read_time();//??????????????????
        // my_tx = (mesh_light_ctl_t *) data.data;
        // ESP_LOGE(MESH_TAG, "my_tx->cmd:%d,my_tx->on:%d,my_tx->token_id:%d,my_tx->token_value:%d", my_tx->cmd,my_tx->on,my_tx->token_id,my_tx->token_value);

        // my_tx_test=(mesh_mac *)data.data;
        // ESP_LOGE(MESH_TAG, "my_tx_test->test_cmd:%d,my_tx_test->test_value:%d,my_tx_test->kid_mac:%s,my_tx_test->father_mac:%s", my_tx_test->test_cmd,my_tx_test->test_value,my_tx_test->kid_mac,my_tx_test->father_mac);
        if (err != ESP_OK || !data.size) 
        {
            ESP_LOGE(MESH_TAG, "err:0x%x, size:%d", err, data.size);
            continue;
        }
        printf("data.size = %d\r\n",data.size);
        child_tx=(mesh_child_to_root *)data.data;

        ESP_LOGE(MESH_TAG, "child_tx->cmd:%d,child_tx->value:%d,child_tx->uart_data:%s", child_tx->cmd,child_tx->value,child_tx->uart_data);
        uart_chid_data=child_tx->uart_data;
        recv_count++;

        if (!(recv_count % 1)) 
        {
            ESP_LOGW(MESH_TAG,
                     "[#RX:%d/%d][L:%d] parent:"MACSTR", receive from "MACSTR", size:%d, heap:%d, flag:%d[err:0x%x, proto:%d, tos:%d]",
                     recv_count, send_count, mesh_layer,
                     MAC2STR(mesh_parent_addr.addr), MAC2STR(from.addr),
                     data.size, esp_get_minimum_free_heap_size(), flag, err, data.proto,
                     data.tos);
        }
       // vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

esp_err_t esp_mesh_comm_p2p_start(void)
{
    static bool is_comm_p2p_started = false;
    if (!is_comm_p2p_started) 
    {
        is_comm_p2p_started = true;
        // xTaskCreate(esp_mesh_p2p_tx_main, "MPTX", 3072, NULL, 5, NULL);
            xTaskCreate(esp_mesh_p2p_rx_main, "MPRX", 3072, NULL, 3, NULL);
            xTaskCreate(wifi_mesh_table_task, "wifi_mesh_table_task", 3072, NULL, 4, NULL);
            xTaskCreate(mqtt_send_task, "mqtt_send_task", 3072, NULL, 4, NULL);

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
        ESP_LOGI(MESH_TAG, "<??????????????????>aid:%d, "MACSTR"",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));
        snprintf(child_mac_str, sizeof(child_mac_str), "%02x%02x-%02x%02x-%02x%02x",child_connected->mac[0],
                    child_connected->mac[1],child_connected->mac[2],child_connected->mac[3],child_connected->mac[4],child_connected->mac[5]);
        strcpy(test_tx_rx.kid_mac, child_mac_str);
        
        ESP_LOGI(MESH_TAG,"child_mac:%s",child_mac_str);
    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, "MACSTR"",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d, layer:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new, mesh_layer);
    }
    break;
    case MESH_EVENT_NO_PARENT_FOUND: {
        mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
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
                 "<??????????????????,????????????>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR", duty:%d",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr), connected->duty);
        last_layer = mesh_layer;
        mesh_connected_indicator(mesh_layer);
        is_mesh_connected = true;
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
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 disconnected->reason);
        is_mesh_connected = false;
        mesh_disconnected_indicator();
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE: {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
        mesh_connected_indicator(mesh_layer);
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS: {
        mesh_event_root_address_t *root_addr = (mesh_event_root_address_t *)event_data;
        ESP_LOGI(MESH_TAG, "<???????????????>root address:"MACSTR"",
                 MAC2STR(root_addr->addr));

        snprintf(root_mac_str, sizeof(root_mac_str), "%02x%02x-%02x%02x-%02x%02x",root_addr->addr[0],
                    root_addr->addr[1],root_addr->addr[2],root_addr->addr[3],root_addr->addr[4],root_addr->addr[5]);
        strcpy(test_tx_rx.father_mac, root_mac_str);
        
        ESP_LOGI(MESH_TAG,"root_mac:%s",root_mac_str);
        
    }
    break;
    case MESH_EVENT_VOTE_STARTED: {
        mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:"MACSTR"",
                 vote_started->attempts,
                 vote_started->reason,
                 MAC2STR(vote_started->rc_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_VOTE_STOPPED>");
        break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
        mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:"MACSTR"",
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
    mqtt_app_start();
    get_nettime_init();
}

void app_main(void)
{
    ESP_ERROR_CHECK(mesh_light_init());
    ESP_ERROR_CHECK(nvs_flash_init());
    /*  tcpip initialization */
    ESP_ERROR_CHECK(esp_netif_init());
    /*  event initialization */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /*  create network interfaces for mesh (only station instance saved for further manipulation, soft AP instance ignored */
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
    ESP_ERROR_CHECK(esp_mesh_set_topology(CONFIG_MESH_TOPOLOGY));
    /*  set mesh max layer according to the topology */
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(CONFIG_MESH_MAX_LAYER));
    ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(1));
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(128));
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
    cfg.channel = CONFIG_MESH_CHANNEL;
    cfg.router.ssid_len = strlen("redyear-nei");
    memcpy((uint8_t *) &cfg.router.ssid, "redyear-nei", cfg.router.ssid_len);
    memcpy((uint8_t *) &cfg.router.password, "12260313",
           strlen("12260313"));
    /* mesh softAP */
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE));
    cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
    cfg.mesh_ap.nonmesh_max_connection = CONFIG_MESH_NON_MESH_AP_CONNECTIONS;
    memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD,
           strlen(CONFIG_MESH_AP_PASSWD));
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
    /* mesh start */
    ESP_ERROR_CHECK(esp_mesh_start());
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
