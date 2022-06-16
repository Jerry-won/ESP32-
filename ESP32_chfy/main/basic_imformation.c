#include "main.h"
static uint8_t mac[6];
long timesTamp_Box = 0;
char device_topic[127];


/*mqtt topic*/
#define Format_JSON_MQTT_TOPIC_RDY "/hl7/%s/data"
#define STEPHAN_JSON_MQTT_TOPIC_RDY "/stephan/%s/data"
// #define DAVID90AC_JSON_MQTT_TOPIC_RDY "/david90ac/%s/data"
#define DAVID_JSON_MQTT_TOPIC_RDY "/david/%s/data"
#define MINDRAY_NB350_JSON_MQTT_TOPIC_RDY "/mindrayNB350/%s/data"
#define MINDRAY_SLE5000_JSON_MQTT_TOPIC_RDY "/mindraySLE5000/%s/data"
uint8_t *getEspMac(void)
{
    esp_efuse_mac_get_default(mac);
    return mac;
}
/**
 * 格式化esp的mac地址
 */
void Get_Format_JSON_ESP32_Info(char *ESP32_Info_Buff)
{
    uint8_t *espMac;
    espMac = getEspMac();
    sprintf(ESP_MAC, "%02x%02x-%02x%02x-%02x%02x", espMac[0], espMac[1], espMac[2], espMac[3], espMac[4], espMac[5]); //拷贝数据
    if(esp_device_type_t == STEPHAN)
    {
        sprintf(device_topic, STEPHAN_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    }
//    else if(esp_device_type_t == DAVID_YP_90AC)
//     {
//         sprintf(device_topic, DAVID90AC_JSON_MQTT_TOPIC_RDY, ESP_MAC);
//     }
    else if(esp_device_type_t == MINDRAY_NB350)
    {
        sprintf(device_topic, MINDRAY_NB350_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    }
    else if(esp_device_type_t == DAVID_YP_90AB||esp_device_type_t == DAVID_YP_90AC)
    {
        sprintf(device_topic, DAVID_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    }
    else if(esp_device_type_t == SLE5000)
    {
        sprintf(device_topic, MINDRAY_SLE5000_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    }
   else
    {
        sprintf(device_topic, Format_JSON_MQTT_TOPIC_RDY, ESP_MAC);
    }
    
    sprintf(ESP32_Info_Buff, Format_JSON_ESP32_Info_RDY, timesTamp_Box, ESP_MAC,is_complete, isOnline,esp_device_type_t,root_mac_str);
}

void Get_Device_Category(Device_TaskType esp_device)
{
    if(esp_device==ESP32_ROOT_NODE)
    {
        esp_device_category_t=NESH_ROOT;
    }
    else if(esp_device_type_t==HX_MINDRAY_MONITOR_EPM12M||esp_device_type_t==HX_MINDRAY_MONITOR_UMEC10)
    {
        esp_device_category_t=MINDRAY_HL7;
    }
    else if(esp_device_type_t==STEPHAN||esp_device_type_t==DAVID_YP_90AC||esp_device_type_t==DAVID_YP_90AB||esp_device_type_t==MINDRAY_NB350||esp_device_type_t==SMITH_JSB)
    {
        esp_device_category_t=UART_DEVICE;
    }
}