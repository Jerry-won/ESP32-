/* Mesh Internal Communication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __MESH_LIGHT_H__
#define __MESH_LIGHT_H__

#include "esp_err.h"

/*******************************************************
 *                Constants
 *******************************************************/
#define MESH_LIGHT_RED       (0xff)
#define MESH_LIGHT_GREEN     (0xfe)
#define MESH_LIGHT_BLUE      (0xfd)
#define MESH_LIGHT_YELLOW    (0xfc)
#define MESH_LIGHT_PINK      (0xfb)
#define MESH_LIGHT_INIT      (0xfa)
#define MESH_LIGHT_WARNING   (0xf9)

#define  MESH_TOKEN_ID       (0x0)
#define  MESH_TOKEN_VALUE    (0xbeef)
#define  MESH_CONTROL_CMD    (0x2)

#define LED_PIN_2 2  //LED 引脚
void LED2_Init();
void LED2_ON();
void LED2_OFF();
void LED2_Flicker();
/*******************************************************
 *                Type 256Definitions
 *******************************************************/

/*******************************************************
 *                Structures
 *******************************************************/
typedef struct {
    uint8_t cmd;
    bool on;
    uint8_t token_id;
    uint16_t token_value;
} mesh_light_ctl_t;

typedef struct
{
    char kid_mac[18];
    char father_mac[18];
    long sntp_time;
}R2C_mesh;//根节点向子节点发送的数据结构体

typedef struct
{
    char device_child_data[1300]; //数据
    char device_child_topic[127]; //每台设备的mqtt主题 通过根节点发送
    int device_datalen;     //数据长度
    int data_count;        //子节点的数据传输次数
}C2R_mesh;//子节点向根节点发送的数据结构体，同时也是根节点解析子节点数据的依据
/*******************************************************
 *                Variables Declarations
 *******************************************************/

/*******************************************************
 *                Function Definitions
 *******************************************************/

void mesh_connected_indicator(int layer);
void mesh_disconnected_indicator(void);

#endif /* __MESH_LIGHT_H__ */
