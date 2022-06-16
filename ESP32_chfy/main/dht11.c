
#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <stdio.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "dht11.h" 
#include "main.h" 

#define DHT11_PIN   21//定义DHT11的引脚
 
#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned short
 

static void InputInitial(void);//设置端口为输入
static void OutputHigh(void);//输出1
static void OutputLow(void);//输出0
static uint8 getData();//读取状态
static void COM(void);    // 温湿写入

//温湿度定义
uchar ucharFLAG,uchartemp;
float Humi,Temp;
uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;
char temp_char[10],huim_char[10];
char name[]={"wl"};
char msg_buf[200]; //发送信息缓冲区
char dataTemplate[] =  "{\"name\":%s\r\n\"temp\":%0.2f\r\n\"hum\":%0.2f\r\n\"net_time\":%ld\r\n}"; //信息模板
//char dataTemplate[] =  "{\"temp\":%0.2f\r\n,\"hum\":%0.2f\r\n,\"net_time\":%ld\r\n}"; //信息模板
char msgJson[100]; //要发送的json格式的数据,75
unsigned short json_len = 0; //json长度




static void InputInitial(void)//设置端口为输入
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
}
 
static void OutputHigh(void)//输出1
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 1);
}
 
static void OutputLow(void)//输出0
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 0);
}
 
static uint8 getData()//读取状态
{
	return gpio_get_level(DHT11_PIN);
}
 
//读取一个字节数据
static void COM(void)    // 温湿写入
{
    uchar i;
    for(i=0;i<8;i++)
    {
        ucharFLAG=2;
        //等待IO口变低，变低后，通过延时去判断是0还是1
        while((getData()==0)&&ucharFLAG++) ets_delay_us(10);
        ets_delay_us(35);//延时35us
        uchartemp=0;
 
        //如果这个位是1，35us后，还是1，否则为0
        if(getData()==1)
          uchartemp=1;
        ucharFLAG=2;
 
        //等待IO口变高，变高后，表示可以读取下一位
        while((getData()==1)&&ucharFLAG++)
          ets_delay_us(10);
        if(ucharFLAG==1)
          break;
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp;
    }
}
 
void Delay_ms(uint16 ms)
{
	int i=0;
	for(i=0; i<ms; i++){
		ets_delay_us(1000);
	}
}
 
void DHT11(void)   //温湿传感启动
{
    OutputLow();
    Delay_ms(19);  //>18MS
    OutputHigh();
    InputInitial(); //输入
    ets_delay_us(30);
    if(!getData())//表示传感器拉低总线
    {
        ucharFLAG=2;
        //等待总线被传感器拉高
        while((!getData())&&ucharFLAG++)
          ets_delay_us(10);
        //等待总线被传感器拉低
        while((getData())&&ucharFLAG++)
          ets_delay_us(10);
        COM();//读取第1字节，
        ucharRH_data_H_temp=ucharcomdata;
        COM();//读取第2字节，
        ucharRH_data_L_temp=ucharcomdata;
        COM();//读取第3字节，
        ucharT_data_H_temp=ucharcomdata;
        COM();//读取第4字节，
        ucharT_data_L_temp=ucharcomdata;
        COM();//读取第5字节，
        ucharcheckdata_temp=ucharcomdata;
        OutputHigh();
        //判断校验和是否一致
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        if(uchartemp==ucharcheckdata_temp)
        {
            //校验和一致，
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
            //保存温度和湿度
            Humi=ucharRH_data_H;
            Humi=((uint16)Humi<<8|ucharRH_data_L)/10;
 
            Temp=ucharT_data_H;
            Temp=((uint16)Temp<<8|ucharT_data_L)/10;
        }
        else
        {
          Humi=100;
          Temp=100;
        }
    }
    else //没成功读取，返回0
    {
    	Humi=0,
    	Temp=0;
    }
 
    OutputHigh(); //输出
}
 

void dht11_read(void)
{     
      read_time();
      DHT11(); //读取温湿度
     // printf("Temp=%.2f--Humi=%.2f%%RH \r\n", Temp,Humi);

      snprintf(msgJson, 90, dataTemplate,name,Temp,Humi,now);
      json_len = strlen(msgJson); 
      memcpy(msg_buf , msgJson, strlen(msgJson)); //在msg_buf中,放入要传的数据msgJson
      msg_buf[strlen(msgJson)] = 0;              //添加一个0作为最后一位, 这样要发送的msg_buf准备好了
      printf("lenth=%d\r\n",strlen(msgJson));
    //  printf("msg_buf=%s\r\n",msg_buf);
      printf("msgJson=%s\r\n",msgJson);

    /*...........下面这种方式也可以构成Json格式数据包..............*/
    
// cJSON *pRoot = cJSON_CreateObject();                         // 创建JSON根部结构体
// cJSON *pValue = cJSON_CreateObject();                        // 创建JSON子叶结构体
// sprintf(temp_char,"%.2f",Temp);
// sprintf(huim_char,"%.2f",Humi);
 //sprintf(now_char,"%ld",now);

// cJSON_AddItemToObject(pRoot,"message",pValue);    // 添加字符串类型数据到根部结构体
// cJSON_AddStringToObject(pValue, "name","wl");
// cJSON_AddStringToObject(pValue,"Humi",huim_char);              // 添加字符串类型数据到子叶结构体
// cJSON_AddStringToObject(pValue,"Temp",temp_char);              // 添加字符串类型数据到子叶结构体
// cJSON_AddStringToObject(pValue,"net_time",now_char);              // 添加字符串类型数据到子叶结构体
// char *sendData = cJSON_Print(pRoot);                        // 从cJSON对象中获取有格式的JSON对象
// printf("data:%s\n\r", sendData);                            // 打印数据

}