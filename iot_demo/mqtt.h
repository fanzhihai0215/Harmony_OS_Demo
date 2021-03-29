#ifndef __MQTT_H__
#define __MQTT_H__
 
int mqtt_init(void);
int mqtt_subscribe(char * topic);
void mqtt_task(char* payload);
void mqtt_exit(void);
 
#endif /* __MQTT_H__ */