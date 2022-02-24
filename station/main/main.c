#include "main.h"
// ┏┓　　　┏┓
// ┏┛┻━━━┛┻┓
// ┃　　　　　　　┃ 　
// ┃　　　━　　　┃
// ┃　┳┛　┗┳　┃
// ┃　　　　　　　┃
// ┃　　　┻　　　┃
// ┃　　　　　　　┃
// ┗━┓　　　┏━┛
// ┃　　　┃ 高山仰止,景行行止.　　　　　　　　
// ┃　　　┃ 虽不能至,心向往之。
// ┃　　　┗━━━┓
// ┃　　　　　　　┣┓
// ┃　　　　　　　┏┛
// ┗┓┓┏━┳┓┏┛
// ┃┫┫　┃┫┫
// ┗┻┛　┗┻┛ 

void app_main(void)
{   
  
  esp_device_type= ESP32_ROOT_NODE; //设备类型
  
    switch (esp_device_type)
    {
    case ESP32_ROOT_NODE: //根节点状态
        wifi_mesh_init();//wifi_mesh初始化，其中包含了sntp功能和mqtt功能
        break;
    case ESP32_CHILD_NODE: //子节点状态
        wifi_mesh_init();//wifi_mesh初始化，其中包含了uart接收功能和mesh发送功能
        break;
    case ESP32_UART_TX_NODE: //uart发送状态
        uart_tx_task();//uart发送任务
        break;
    default:
        break;
    }
}
// idf.py -p COM3 flash monitor
/*注：1.编译之前应先修改编译路径
      2.在烧录该代码至单片机之前应修改好WiFi配置，在wifi_mesh.c中修改ssid和password即可
      3.在连接mqtt服务器时应正确订阅主题
      4.若发现客户端接收的网络时间是两位十进制数，则应更换一个WiFi热点且尽量使用2.4G网络热点，网络时间的正确格式应是十位十进制数*/
