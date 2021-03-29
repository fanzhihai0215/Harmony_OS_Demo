#include<stdio.h>
#include<unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define PWM_FREQ_DIVITION 64000
#define ADC_RESOLUTION 4996

static void ColorfulLightTask(void *arg)
{
    (void)arg;
    //由于炫彩灯板中红色led灯的GIPO为10，这里初始化控制GPIO10的控制方式为PWM模式
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10,WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);

    //调用函数初始化PWM模式
    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
    
   while(1){
       unsigned short data = 0;
       unsigned short duty = 0;
       //使用AdcRead函数对3通道进行ADC信号读取，读取到的结果存储在data中
       if(AdcRead(WIFI_IOT_ADC_CHANNEL_3,&data,WIFI_IOT_ADC_EQU_MODEL_4,WIFI_IOT_ADC_CUR_BAIS_DEFAULT,0)==WIFI_IOT_SUCCESS)
       {
           printf("data:%d",data);
           duty = PWM_FREQ_DIVITION *(unsigned int)data / ADC_RESOLUTION;
           //128 1820
            //duty = PWM_FREQ_DIVITION * (1948-(unsigned int)data) / ADC_RESOLUTION;

       }
       //PWM模式开启对红色led灯的控制
       PwmStart(WIFI_IOT_PWM_PORT_PWM1,duty,PWM_FREQ_DIVITION);
       usleep(10000);
       //PWM模式关闭对红色led灯的控制
       PwmStop(WIFI_IOT_PWM_PORT_PWM1);

   }
}
static void ColorfulLightDemo(void)
{
    osThreadAttr_t attr;
    GpioInit();
    attr.name="ColorfulLightTask";
    attr.attr_bits=0U;
    attr.cb_mem=NULL;
    attr.cb_size=0U;
    attr.stack_mem=NULL;
    attr.stack_size=4096;
    attr.priority=osPriorityNormal;

    if(osThreadNew(ColorfulLightTask,NULL,&attr) == NULL){
        printf("[ColorfulLightDemo] Failed to create ColorfulLightTask!\n");
    }

}
APP_FEATURE_INIT(ColorfulLightDemo);
