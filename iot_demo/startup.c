#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"

#include "cJSON.h"

#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "lwip/api_shell.h"

#include "MQTTPacket.h"
#include "transport.h"

#include "wifi_connect.h"
#include "mqtt.h"
#include "aht20.h"

#include "wifi_device.h"

void HelloWorld(void)
{
    printf("[DEMO] Start Up.\n");
    WifiConnect();
    osDelay(1000);

    printf("[DEMO] Start MQTT\r\n");
    if(mqtt_init() == 1){
        printf("[DEMO]MQTT Connect\r\n");
        mqtt_subscribe("testtopic");        //设置订阅
        while (1) {
            //读取温湿度
            uint32_t retval = 0;

            IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
            IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

            I2cInit(WIFI_IOT_I2C_IDX_0, 400*1000);

            retval = AHT20_Calibrate();
            printf("AHT20_Calibrate: %d\r\n", retval);

            float temp = 0.0, humi = 0.0;

            retval = AHT20_StartMeasure();
            printf("AHT20_StartMeasure: %d\r\n", retval);

            retval = AHT20_GetMeasureResult(&temp, &humi);
            printf("AHT20_GetMeasureResult: %d, temp = %.2f, humi = %.2f\r\n", retval, temp, humi);

            sleep(1);
            //生成json
            char strTemp[8], strHumi[8];
            sprintf(strTemp, "%.2f", temp);
            sprintf(strHumi, "%.2f", humi);
            cJSON *root = cJSON_CreateObject();
            cJSON_AddItemToObject(root, "temp", cJSON_CreateString(strTemp));
            cJSON_AddItemToObject(root, "humi", cJSON_CreateString(strHumi));
            char *out = cJSON_Print(root);
            printf("%s \n", out);

            mqtt_task(out);
            if(out)
            {
                free(out);
            }   
            if(root)
            {
                cJSON_Delete(root);
            }

            osDelay(1000);
        }
    }
}
SYS_RUN(HelloWorld);