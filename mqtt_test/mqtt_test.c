#include <stdio.h>
 
#include <unistd.h>
 
#include "ohos_init.h"
#include "cmsis_os2.h"
 
#include <unistd.h>
#include "hi_wifi_api.h"
//#include "wifi_sta.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
 
#include "lwip/sockets.h"
 
#include "MQTTPacket.h"
#include "transport.h"
 
int mqtt_rc = 0;
int mqtt_sock = 0;
int mqtt_len = 0;
unsigned char mqtt_buf[200];
int mqtt_buflen = sizeof(mqtt_buf);
int mqtt_req_qos = 0;
int mqtt_msgid = 1;
int toStop = 0;
MQTTString topicString = MQTTString_initializer;
 
void mqtt_exit(void){
	transport_close(mqtt_sock);
	mqtt_rc = mqtt_rc;
	printf("[MQTT] ERROR EXIT\n");
}
 
void mqtt_task(void){
	while (!toStop){
		char* payload = "{\"temp\":28}";
		int payloadlen = strlen(payload);
 
		if (MQTTPacket_read(mqtt_buf, mqtt_buflen, transport_getdata) == PUBLISH){
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char* payload_in;
			int rc;
			MQTTString receivedTopic;
			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
					&payload_in, &payloadlen_in, mqtt_buf, mqtt_buflen);	// 发送数据
			printf("message arrived %.*s\n", payloadlen_in, payload_in);
 
            mqtt_rc = rc;
        }
 
		printf("publishing reading\n");
		mqtt_len = MQTTSerialize_publish(mqtt_buf, mqtt_buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
        mqtt_rc = transport_sendPacketBuffer(mqtt_sock, mqtt_buf, mqtt_len);
 
		osDelay(1000);
	}
}
 
int mqtt_subscribe(char * topic){	// MQTT订阅
	/* subscribe */
	topicString.cstring = topic;
	mqtt_len = MQTTSerialize_subscribe(mqtt_buf, mqtt_buflen, 0, mqtt_msgid, 1, &topicString, &mqtt_req_qos);	// MQTT订阅
	mqtt_rc = transport_sendPacketBuffer(mqtt_sock, mqtt_buf, mqtt_len);				// 传输发送缓冲区
	if (MQTTPacket_read(mqtt_buf, mqtt_buflen, transport_getdata) == SUBACK) 	/* wait for suback */	// 等待订阅返回
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;
 
		mqtt_rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_buf, mqtt_buflen);
		if (granted_qos != 0){
			printf("granted qos != 0, %d\n", granted_qos);
			mqtt_exit();
			return 0;
		}
 
		return 1;
	}else{
		mqtt_exit();
		return 0;
	}
}
 
int mqtt_init(void){		// MQTT初始化开始连接
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	// char *host = "183.230.40.96";		// 地址
	char *host = "196.222.222.30";		// 地址
	int port = 1883;					// 端口
 
	
	mqtt_sock = transport_open(host, port);
	if(mqtt_sock < 0){
		return mqtt_sock;
	}
 
	data.clientID.cstring = "device1";			// ClientID
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	// data.username.cstring = "406453";		// 用户名
	// data.password.cstring = "version=2018-10-31&res=products%2F406453%2Fdevices%2Fdevice1&et=1614933163&method=sha1&sign=oATZY1AeeFUGanhzBamvELRmEz0%3D";	// 密码
 
	printf("[MQTT]Sending to hostname %s port %d\n", host, port);
 
	mqtt_len = MQTTSerialize_connect(mqtt_buf, mqtt_buflen, &data);		// 开始连接
	mqtt_rc = transport_sendPacketBuffer(mqtt_sock, mqtt_buf, mqtt_len);		// 发送缓冲区
 
	if (MQTTPacket_read(mqtt_buf, mqtt_buflen, transport_getdata) == CONNACK){	// 等待链接返回
		unsigned char sessionPresent, connack_rc;
 
		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_buf, mqtt_buflen) != 1 || connack_rc != 0){
			printf("Unable to connect, return code %d\n", connack_rc);
			mqtt_exit();
			return 0;
		}
	}else{
		mqtt_exit();
		return 0;
	}
 
	return 1;
}
 
 
void mqtt_test(void){
	printf("[MQTT]Start MQTT\r\n");
    if(mqtt_init() == 1){
		printf("[MQTT]MQTT Connect\r\n");
		mqtt_subscribe("testtopic");		//设置订阅
		mqtt_task();
	}
}
// SYS_RUN(mqtt_test);