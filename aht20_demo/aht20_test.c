#include "aht20_demo.h"

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"

void Aht20TestTask(void* arg)
{
    (void) arg;
    uint32_t retval = 0;

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    I2cInit(WIFI_IOT_I2C_IDX_0, 400*1000);

    retval = AHT20_Calibrate();
    printf("AHT20_Calibrate: %d\r\n", retval);

    while (1) {
        float temp = 0.0, humi = 0.0;

        retval = AHT20_StartMeasure();
        printf("AHT20_StartMeasure: %d\r\n", retval);

        retval = AHT20_GetMeasureResult(&temp, &humi);
        printf("AHT20_GetMeasureResult: %d, temp = %.2f, humi = %.2f\r\n", retval, temp, humi);

        sleep(1);
    }
}

void Aht20Test(void)
{
    osThreadAttr_t attr;

    attr.name = "Aht20Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(Aht20TestTask, NULL, &attr) == NULL) {
        printf("[Aht20Test] Failed to create Aht20TestTask!\n");
    }
}
APP_FEATURE_INIT(Aht20Test);