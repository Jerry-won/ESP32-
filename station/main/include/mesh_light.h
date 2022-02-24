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
    uint8_t test_cmd;
    uint8_t test_value;
    char kid_mac[18];
    char father_mac[18];
}mesh_mac;

typedef struct
{
    uint8_t cmd;
    uint8_t value;
    uint8_t uart_data[256];
    // char *uart_data;
}mesh_child_to_root;
/*******************************************************
 *                Variables Declarations
 *******************************************************/

/*******************************************************
 *                Function Definitions
 *******************************************************/

void mesh_connected_indicator(int layer);
void mesh_disconnected_indicator(void);

#endif /* __MESH_LIGHT_H__ */