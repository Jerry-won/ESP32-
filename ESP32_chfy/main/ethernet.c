/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "ethernet.h"
#include "main.h"
static const char *TAG = "eth_example";
extern Whether_Online isOnline;
TaskHandle_t Mindray_Monitor_Epm10_TaskHandle = NULL;
TaskHandle_t HX_Mindray_Monitor_EPM12M_TX_Task_TaskHandle = NULL;
TaskHandle_t HX_Mindray_Monitor_UMEC10_TX_TaskHandle = NULL;
TaskHandle_t HX_Mindray_Monitor_UMEC10_RX_TaskHandle = NULL;
/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) 
    {
    case ETHERNET_EVENT_CONNECTED:
        /*设置以太网静态地址*/
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
        tcpip_adapter_ip_info_t eth;
        if(esp_device_type_t== HX_MINDRAY_MONITOR_EPM12M)//眉山迈瑞监护仪EPM12M
        {
            eth.ip.addr = ipaddr_addr("192.168.3.10");//IP地址
        }
        else
        {
            eth.ip.addr = ipaddr_addr("192.168.3.11");//IP地址
        }

        eth.netmask.addr = ipaddr_addr("255.255.255.0");//子网掩码
        eth.gw.addr = ipaddr_addr("192.168.3.1");//网关地址
        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &eth);

        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        isOnline = NOT_LINE;
        vTaskSuspend(HX_Mindray_Monitor_UMEC10_RX_TaskHandle);
        vTaskSuspend(HX_Mindray_Monitor_UMEC10_TX_TaskHandle);
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        vTaskSuspend(HX_Mindray_Monitor_UMEC10_RX_TaskHandle);
        vTaskSuspend(HX_Mindray_Monitor_UMEC10_TX_TaskHandle);
        isOnline = NOT_LINE;
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    // ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    // const esp_netif_ip_info_t *ip_info = &event->ip_info;

    // ESP_LOGI(TAG, "Ethernet Got IP Address");
    // ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    // ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    // ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    
    if(esp_device_type_t == HX_MINDRAY_MONITOR_EPM12M)//服务端
    {
        xTaskCreatePinnedToCore(HX_Mindray_Monitor_EPM12M_TX_Task, "HX_Mindray_Monitor_EPM12M_TX_Task", 10000, NULL, 3, &HX_Mindray_Monitor_EPM12M_TX_Task_TaskHandle, 0);
    }
    if(esp_device_type_t == HX_MINDRAY_MONITOR_UMEC10)//客户端
    {
       if(!HX_Mindray_Monitor_UMEC10_RX_TaskHandle)
       {
           xTaskCreatePinnedToCore(HX_Mindray_Monitor_UMEC10_RX_Task, "HX_Mindray_Monitor_UMEC10_RX_Task", 10000, NULL, 3, &HX_Mindray_Monitor_UMEC10_RX_TaskHandle, 0);
       }
       else
       {
           vTaskResume(HX_Mindray_Monitor_UMEC10_RX_TaskHandle);
       }
       if(!HX_Mindray_Monitor_UMEC10_TX_TaskHandle)
       {
           xTaskCreatePinnedToCore(HX_Mindray_Monitor_UMEC10_TX_Task, "HX_Mindray_Monitor_UMEC10_TX_Task", 10000, NULL, 4, &HX_Mindray_Monitor_UMEC10_TX_TaskHandle, 0);
       }
       else
       {
           vTaskResume(HX_Mindray_Monitor_UMEC10_TX_TaskHandle);
       } 
    }

    
}

void ethernet_init(void)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = -1;//-1
// #if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num = 23;
    mac_config.smi_mdio_gpio_num = 18;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    
    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}
