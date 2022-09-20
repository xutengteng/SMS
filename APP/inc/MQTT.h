#ifndef _MQTT_H_
#define _MQTT_H_

#include <stdbool.h>
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "function.h"

#define KEY_ENABLE  1
#define KEY_DISABLE 0

EventGroupHandle_t app_event_key;
/**【主题】
* @发布：/${ProductKey}/${deviceNo}/user/update 
* @响应：/${ProductKey}/${deviceNo}/user/updateResp

* @订阅：/${ProductKey}/${deviceNo}/user/set 
* @响应：/${ProductKey}/${deviceNo}/user/setResp 

* @心跳：/${ProductKey}/${deviceNo}/user/heartbeat
* @心跳响应：/${ProductKey}/${deviceNo}/user/heartbeatResp

* @ProductKey ：为 20 位 BE 码前 9 个字符 
* @${deviceNo}： 为设备序列号，统一固定为 20 位 BE 码字符，BE 码的最后 8 位字符作为设备的 ID 号，$代表可变
* @user ： 默认分类
* @update: 为向服务器发布数据 
* @set: 为向服务器订阅设置消息
*/

/*【ID】
16A 账号BE17C9M0100000000001  密码75622e04c987d9f6
10A 账号BE17C8M0100000000001 密码8caf5408d0cdaaef
*/

/*主题定义*/
typedef struct topic
{
    char SUB_TOPIC[41];
    char SUB_RESP_TOPIC[45];
    char PUB_TOPIC[44];
    char PUB_RESP_TOPIC[48];
    char BEAT_TOPIC[47];
    char BEAT_RESP_TOPIC[51];
}topic;

void app_comm_init();

void mqtt_subscribed_process(char* res_topic,char* res_message);
int  app_mqtt_client_publish(const char * topic, const char * publish_string,int len ,int qos);

void app_pub_beat();
int  mqtt_publish_data(power power_s);
void mqtt_publish_warn(uint8_t alarm);

void app_mqtt_client_disconnect();
/**【Qos】
* Qos1：AtLeastOne 至少一次到达。 
*/


/**【Message数据帧】
*【9E】 【BF】 【Length】 【Type】 【Cmd】 【Data】 【CheckSum】 【EE】 【BA】
*						  【__________帧长计算范围___________】
*			   【_______校验和计算范围___________】
*
*【9E】 【BF】: 2 个字节，为帧头;
*【Length】:	2 个字节，帧长，帧长的计算范围是从 Type（包含）到 CheckSum（包含）;
*【Cmd】：      指令类型;
*【Data】:		数据内容;
*【CheckSum】:	数据累加校验和 CheckSum（低 8 位）= Length+Type+Data[0]+ Data[1]…Data[ i ];
*【EE】 【BA】: 帧尾。
*/

/**【Type】
*【0x21】发布 √
*【0x22】Server响应
*【0xA1】订阅
*【0xA2】Device响应 √
*/

/**【Cmd】
*【0x01】心跳
*【0x13】采集数据
*【0x17】预警
*【0x15】下发控制
*【0x04】修改ip、port
*【0x01】修改采集频率
*/

/**【数据格式规则】
Mqtt 协议版本：3.11 
安全认证方式：产品 Key+产品 Secrect。可通过 TLS/DTLS （支持单双向认证）保证网络传输安全。 
默认的 Qos 级别 Qos1：AtLeastOne 至少一次到达。 
服务器订阅：SslProtocols.Tls12 
服务器端口号：1777 
数据内容格式：字节数组。适用于向服务器的发布和从服务器的订阅 
服务端连接规则：服务端对连接使用持续会话，暂不清理。 
编码格式：UTF8 
若数据是字符串类型，该字符串必须放在双引号中。 
多字节内容，高字节在前
*/

/**【注】
1.MQTT 用户名密码，设备端具有唯一性，设备出厂前被写入，由软件生成 
2.协议中不确定长度的字符串，用双引号包含，依据双引号间的内容确定字符串 
3.获取不到的字符串用“” 
4.获取不到的十六进制数用 0xFF 补齐 
5.测试服务器地址：tcp://msgtest.haierbiomedical.com:1777 
6.测试服务器 MQTT 用户名：haierTest 密码：haier 
*/

#endif