#ifndef __WIFI_MESH_H__
#define __WIFI_MESH_H__
void wifi_mesh_init(void);
void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
esp_err_t esp_mesh_comm_p2p_start(void);
void esp_mesh_p2p_root_rx(void *arg);
void esp_mesh_C2R_tx_main(void *arg);
void mqtt_send_task(void *arg);                                              
void wifi_mesh_table_task(void *arg);
void send_wifi_mesh_table();


#endif