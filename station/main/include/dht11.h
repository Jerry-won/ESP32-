#ifndef __DHT11_H__
#define __DHT11_H__

#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned short


void Delay_ms(uint16 ms);
void DHT11(void);   //温湿传感启动
void dht11_read(void);

extern char msg_buf[200]; //发送信息缓冲区
extern char msgJson[100]; //要发送的json格式的数据

#endif