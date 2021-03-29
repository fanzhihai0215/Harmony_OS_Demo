#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
 
#define LED_ON_TIME_US 2000000
#define LED_OFF_TIME_US 1000000
 
static void LedExampleEntry(void)
{
    //GPIO引脚初始化
    GpioInit();
    //设置管脚功能为GPIO
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    //设置GPIO引脚方向为输出
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    while (1) {
        //设置GPIO09输出为低电平
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
        //打印输出日志
        printf("[DEMO] LED on.n");
        //延时一定时间，即亮灯的持续时间
        usleep(LED_ON_TIME_US);
        //设置GPIO09输出为高电平
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
        //打印输出日志
        printf("[DEMO] LED off.n");
        //延时一定时间，即灭灯的持续时间
        usleep(LED_OFF_TIME_US);
    }
}
SYS_RUN(LedExampleEntry);
