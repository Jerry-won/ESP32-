#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
// #include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
int tcp_client_init(void);
void HX_Mindray_Monitor_UMEC10_RX_Task(void *asg);
void HX_Mindray_Monitor_UMEC10_TX_Task(void *client_sock);
void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen);
void Monitor_UMEC10_Data_Form_Send(char *data, int len);
char Monitor_UMEC10_RX_Buffer[1024 * 1];
char Monitor_UMEC10_RX_Temp[256 * 1];
#define Mindray_Monitor_Format_JSON_IPM8_GET_DATA "MSH|^~\\&|||||||QRY^R02|1203|P|2.3.1\rQRD|20060731145557|R|I|Q895211|||||RES\rQRF|MON||||0&0^1^1^0^101&102&103&104\r"
// #define Mindray_Monitor_Format_JSON_IPM8_GET_DATA "MSH|^~\\&|||||||QRY^R02|1203|P|2.3.1\rQRD|20060731145557|R|I|Q895211|||||RES\rQRF|MON||||0&0^1^1^0^101&500&501&502\rQRF|MON||||0&0^1^1^0^160&161&220&222\rQRF|MON||||0&0^1^1^0^170&171&172&151\rQRF|MON||||0&0^1^1^0^503&504&505\rQRF|MON||||0&0^1^1^0^200&201&202&203\r"
// #define Mindray_Monitor_Format_JSON_IPM8_GET_DATA "MSH|^~\\&|||||||QRY^R02|1203|P|2.3.1\rQRD|20060731145557|R|I|Q895211|||||RES\rQRF|MON||||0&0^1^1^0^101&102&103&104\rQRF|MON||||0&0^1^1^0^151&160&170&171\rQRF|MON||||0&0^1^1^0^172\r"
// #define Mindray_Monitor_Format_JSON_IPM8_GET_DATA "MSH|^~\\&|||||||QRY^R02|1203|P|2.3.1\rQRD|20060731145557|R|I|Q895211|||||RES\r QRF|MON||||0&0^1^1^0^101&102&103&104\rQRF|MON||||0&0^1^1^0^151&160&170&171\rQRF|MON||||0&0^1^1^0^172\r"

#endif