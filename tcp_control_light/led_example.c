/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include <unistd.h>
#include<gtk/gtk.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#define LED_INTERVAL_TIME_US 300000
#define LED_TASK_STACK_SIZE 512
#define LED_TASK_PRIO 25

enum LedState {
    LED_ON = 0,
    LED_OFF,
    LED_SPARK,
};

enum LedState g_ledState = LED_SPARK;

static void *LedTask(const char *arg)
{
    (void)arg;
    while (1) {
        switch (g_ledState) {
            case LED_ON:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
                usleep(LED_INTERVAL_TIME_US);
                break;
            case LED_OFF:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
                usleep(LED_INTERVAL_TIME_US);
                break;
            case LED_SPARK:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
                usleep(LED_INTERVAL_TIME_US);
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
                usleep(LED_INTERVAL_TIME_US);
                break;
            default:
                usleep(LED_INTERVAL_TIME_US);
                break;
        }
    }

    return NULL;
}

static void OnButtonPressed(char *arg)
{
    (void) arg;

    enum LedState nextState = LED_SPARK;
    switch (g_ledState) {
        case LED_ON:
            nextState = LED_OFF;
            break;
        case LED_OFF:
            nextState = LED_ON;
            break;
        case LED_SPARK:
            nextState = LED_OFF;
            break;
        default:
            break;
    }

    g_ledState = nextState;
}

static void LedExampleEntry(void)
{
    osThreadAttr_t attr;
    GtkWidget * window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    111
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        OnButtonPressed, NULL);

    attr.name = "LedTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LED_TASK_STACK_SIZE;
    attr.priority = LED_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)LedTask, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create LedTask!\n");
    }
}

SYS_RUN(LedExampleEntry);