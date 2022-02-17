1、准备三块esp32芯片，杜邦线， 创建工程名 esp32_名字首字母拼音缩写 ，如 esp32_tfy ，考核截至时间为2022-2-16(星期三) 下午15:00
2、一块作为wifi-mesh的根节点，同时要使用mqtt并获取sntp的网络时间，把获取到的时间戳发送到子节点
3、一块作为wifi-mesh的子节点，同时作为UART的接收端（UART的参数在后面），将接收到的数据和其它的数据构建出json，通过根节点的MQTT发送出去，需要的json数据格式如下。
{
    "deviceInfo":{
        "name":"tfy",                                   //自己的名字拼音首字母缩写
        "timesTamp_Box":1644369591,     //时间戳
        "box_code":"8cee-1323-as42",    //当前esp32的mac地址 格式像我这样
        "root_mac":"8cee-4567-advf"     //根节点的mac地址
    },
    "deviceData":"通过串口接收到的数据"
}
4、最后一块esp32作为UART的数据发送端，不需要mesh，每隔1发送一次数据，需要发送的数据如下
MSH|~&|||||||ORUR01|503|P|2.3.1|
OBX||NM|171Dia|2105|-100||||||F||APERIODIC|00000000000000
OBX||NM|172Mean|2105|-100||||||F||APERIODIC|00000000000000
OBX||NM|170Sys|2105|-100||||||F||APERIODIC|00000000000000
OBX||NM|173NIBP_PR|2105|-100||||||F||APERIODIC|00000000000000
注意，每次要发送上面所有的数据 

5、UART的参数
波特率 192000 + 8位数据位+奇校验位 + 1位停止位
TXD：GPIO4
RXD：GPIO5
6、将最后在emqx中收到的json数据 ，格式化成功后截图放在工程中一并上传,效果如下
Topic: hl7/7c9e-bdf3-dabc/dataQoS: 0
{
  "deviceInfo": {
    "name": "wl",
    "timesTamp_Box": "1644982996",
    "box_code": "",
    "root_mac": "7c9e-bdf3-d841"
  }
}
