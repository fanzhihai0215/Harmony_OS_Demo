#include<stdio.h>
#include<unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define HUMAN_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_3

#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_11_GPIO

#define BLUE_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_12
#define BLUE_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_12_GPIO

#define LED_DELAY_TIME_US 300000
#define LED_BRIGHT WIFI_IOT_GPIO_VALUE1
#define LED_DARK WIFI_IOT_GPIO_VALUE0

#define NUM_BLINKS 2

#define PWM_FREQ_DIVITION 64000
#define ADC_RESOLUTION 4996

static void ColorfulLightTask(void *arg)
{
    (void)arg;
    IoSetFunc(RED_LED_PIN_NAME,WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    IoSetFunc(GREEN_LED_PIN_NAME,WIFI_IOT_IO_FUNC_GPIO_11_PWM2_OUT);
    IoSetFunc(BLUE_LED_PIN_NAME,WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
    PwmInit(WIFI_IOT_PWM_PORT_PWM2);
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);
    
   while(1){
       unsigned short data = 0;
       unsigned short duty = 0;
       if(AdcRead(WIFI_IOT_ADC_CHANNEL_4,&data,WIFI_IOT_ADC_EQU_MODEL_4,WIFI_IOT_ADC_CUR_BAIS_DEFAULT,0)==WIFI_IOT_SUCCESS)
       {
           printf("data:%d",data);
           //128 1820
           duty = PWM_FREQ_DIVITION * (1948-(unsigned int)data) / ADC_RESOLUTION;

       }
       PwmStart(WIFI_IOT_PWM_PORT_PWM2,duty,PWM_FREQ_DIVITION);
       usleep(10000);
       PwmStop(WIFI_IOT_PWM_PORT_PWM2);

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
