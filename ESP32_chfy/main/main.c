#include "main.h"
/***
 *                    _ooOoo_
 *                   o8888888o
 *                   88" . "88
 *                   (| -_- |)
 *                    O\ = /O
 *                ____/`---'\____
 *              .   ' \\| |// `.
 *               / \\||| : |||// \
 *             / _||||| -:- |||||- \
 *               | | \\\ - /// | |
 *             | \_| ''\---/'' | |
 *              \ .-\__ `-` ___/-. /
 *           ___`. .' /--.--\ `. . __
 *        ."" '< `.___\_<|>_/___.' >'"".
 *       | | : `- \`.;`\ _ /`;.`/ - ` : | |
 *         \ \ `-. \_ __\ /__ _/ .-` / /
 * ======`-.____`-.___\_____/___.-`____.-'======
 *                    `=---='
 * **************电子佛压制一切仿生BUG*****************/
 /***************佛祖保佑 永无BUG*************/

void app_main(void)
{   
    esp_device_type_t = ESP32_ROOT_NODE; //设备类型
    esp_err_t ret = nvs_flash_init();//非易失性存储初始化
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
      /* 初始化底层TCP/IP堆栈。在应用程序启动时，应该调用此函数一次。*/
    ESP_ERROR_CHECK(esp_netif_init());
    /* 创建默认事件循环 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    Get_Device_Category(esp_device_type_t);//对设备进行分类
    LED2_Init();
    /*开机闪烁确保程序正在执行，若未闪烁请按复位键或重插电源线*/
    LED2_ON();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    LED2_OFF();
    switch (esp_device_category_t)
    {
    case NESH_ROOT: //根节点
        wifi_mesh_init(); //wifi_mesh初始化，其中包含了sntp功能和mqtt功能
        break;
    case MINDRAY_HL7: //迈瑞监护仪
        tcpip_adapter_init();
        ethernet_init();
        wifi_mesh_init();
        break;
    case UART_DEVICE: //UART类型设备
        uart_start();
        wifi_mesh_init();
        break;  
    default:
        break;
    }
}
// idf.py -p COM3 flash monitor
/*注：1.编译之前应先修改编译路径
      2.在烧录该代码至单片机之前应修改好WiFi配置，在wifi_mesh.c中修改ssid和password即可
      3.在连接mqtt服务器时应更改MQTT服务器IP且须正确订阅主题
      4.若发现客户端接收的网络时间是两位十进制数，则应更换WiFi热点或sntp IP且必须使用2.4G网络热点，网络时间的正确格式应是十位十进制数*/
