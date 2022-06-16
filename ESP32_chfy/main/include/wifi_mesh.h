#ifndef __WIFI_MESH_H__
#define __WIFI_MESH_H__
void wifi_mesh_init(void);
void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
esp_err_t esp_mesh_comm_p2p_start(void);
void esp_mesh_p2p_root_rx(void *arg);
void esp_mesh_p2p_tx_main(void *arg);
void mqtt_send_task(void *arg);                                              
void wifi_mesh_table_task(void *arg);
void send_wifi_mesh_table();
void esp_mesh_p2p_child_rx(void *arg);//子节点接收根节点数据
void esp_mesh_p2p_child_tx(void *arg);//子节点向根节点发送
void wifi_mesh_send_data(char *epm12m_data, int len, char *topic);
extern char root_mac_str[18];

// #define Format_JSON_STEPHAN_Info "{\"deviceInfo\":{\"name\":\"%s\",\"box_code\": \"%s\", \"topic\":\"%s\"},\"nodelist\":[{\"root_mac\":\"%s\",\"timesTamp_Box\":%ld,\"data\":\"%s\"}]}"

#endif