#ifndef __BASIC_IMFORMATION_H__
#define __BASIC_IMFORMATION_H__

#define Format_JSON_ESP32_AND_Mindray_Monitor_RDY "{%s,\r\n\"deviceData\":\"%s\"\r\n}"

/* ESP32的信息 */
#define Format_JSON_ESP32_Info_RDY "\"deviceInfo\":{\"timesTamp_Box\":%lu,\"esp_mac\":\"%s\",\"is_complete\":%d,\"isOnline\":%d,\"device_type\":%d,\"root_mac\":\"%s\"}"
typedef enum
{
  NOT_LINE, //设备不在线
  ON_LINE   //设备在线
} Whether_Online;

char ESP_MAC[18]; /* ESP32的mac地址 */
extern char device_topic[127];
extern long timesTamp_Box;
uint8_t *getEspMac(void);
void Get_Format_JSON_ESP32_Info(char *ESP32_Info_Buff);
void Get_Device_Category(esp_device_type_t);
Whether_Online isOnline;

#endif