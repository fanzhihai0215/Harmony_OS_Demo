/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"

#include <sys/types.h>  // for AF_INET SOCK_STREAM
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h>  // for inet_pton
#include "lwip/sockets.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define PARAM_SERVER_ADDR "196.222.222.30"
#define RED_ON "redOn\0"
#define RED_OFF "redOff\0"
#define GREEN_ON "greenOn\0"
#define GREEN_OFF "greenOff\0"
#define RELLOW_ON "yellowOn\0"
#define RELLOW_OFF "yellowOff\0"
#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_11_GPIO

#define RELLOW_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_12
#define RELLOW_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_12_GPIO
#define LED_BRIGHT WIFI_IOT_GPIO_VALUE1
#define LED_DARK WIFI_IOT_GPIO_VALUE0
// static char request[] = "Hello";
static char response[128] = "";

static void PrintLinkedInfo(WifiLinkedInfo *info)
{
    if (!info)
        return;

    static char macAddress[32] = {0};
    unsigned char *mac = info->bssid;
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("bssid: %s, rssi: %d, connState: %d, reason: %d, ssid: %s\r\n",
           macAddress, info->rssi, info->connState, info->disconnectedReason, info->ssid);
}

static int g_connected = 0;
static void OnWifiConnectionChanged(int state, WifiLinkedInfo *info)
{
    if (!info)
        return;

    printf("%s %d, state = %d, info = \r\n", __FUNCTION__, __LINE__, state);
    PrintLinkedInfo(info);

    if (state == WIFI_STATE_AVALIABLE)
    {
        g_connected = 1;
    }
    else
    {
        g_connected = 0;
    }
}

static void OnWifiScanStateChanged(int state, int size)
{
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
}

static void WifiConnectTask(void *arg)
{
    (void)arg;
    WifiErrorCode errCode;
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = OnWifiConnectionChanged,
        .OnWifiScanStateChanged = OnWifiScanStateChanged};
    WifiDeviceConfig apConfig = {0};
    int netId = -1;

    osDelay(10);
    errCode = RegisterWifiEvent(&eventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);

    // setup your AP params
    strcpy(apConfig.ssid, "BOARD");
    strcpy(apConfig.preSharedKey, "BOARD123456a?");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    // while (1) {
    errCode = EnableWifi();
    printf("EnableWifi: %d\r\n", errCode);
    osDelay(10);

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("AddDeviceConfig: %d\r\n", errCode);

    g_connected = 0;
    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);

    while (!g_connected)
    {
        osDelay(10);
    }
    printf("g_connected: %d\r\n", g_connected);
    osDelay(50);

    // 联网业务开始
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        err_t ret = netifapi_dhcp_start(iface);
        printf("netifapi_dhcp_start: %d\r\n", ret);

        osDelay(200); // wait DHCP server give me IP
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("netifapi_netif_common: %d\r\n", ret);
    }
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    unsigned short port = 33333;
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;   // AF_INET表示IPv4协议
    serverAddr.sin_port = htons(port); // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, PARAM_SERVER_ADDR, &serverAddr.sin_addr) <= 0)
    { // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }

    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回 -1
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("connect failed!\r\n");
        goto do_cleanup;
    }
    printf("connect to server %s success!\r\n", PARAM_SERVER_ADDR);

    // 建立连接成功之后，这个TCP socket描述符 —— sockfd 就具有了 “连接状态”，发送、接收 对端都是 connect 参数指定的目标主机和端口
    // retval = send(sockfd, request, sizeof(request), 0);
    // if (retval < 0) {
    //     printf("send request failed!\r\n");
    //     goto do_cleanup;
    // }
    // printf("send request{%s} %ld to server done!\r\n", request, retval);
    while (1)
    {

        retval = recv(sockfd, &response, sizeof(response), 0);
        if (retval <= 0)
        {
            printf("send response from server failed or done, %ld!\r\n", retval);
            goto do_cleanup;
        }
        response[retval] = '\0';
        printf("recv response{%s} %ld from server done!\r\n", response, retval);
        if (strncmp(RED_ON, response, 5) == 0)
        {
            printf("red on----------------");
            GpioSetOutputVal(RED_LED_PIN_NAME, LED_BRIGHT);
        }
        else if (strncmp(RED_OFF, response, 6) == 0)
        {
            printf("red off--------------");
            GpioSetOutputVal(RED_LED_PIN_NAME, LED_DARK);
        }
        else if (strncmp(GREEN_ON, response, 7) == 0)
        {
            printf("red on----------------");
            GpioSetOutputVal(GREEN_LED_PIN_NAME, LED_BRIGHT);
        }
        else if (strncmp(GREEN_OFF, response, 8) == 0)
        {
            printf("red off--------------");
            GpioSetOutputVal(GREEN_LED_PIN_NAME, LED_DARK);
        }
        else if (strncmp(RELLOW_ON, response, 8) == 0)
        {
            printf("red on----------------");
            GpioSetOutputVal(RELLOW_LED_PIN_NAME, LED_BRIGHT);
        }
        else if (strncmp(RELLOW_OFF, response, 9) == 0)
        {
            printf("red off--------------");
            GpioSetOutputVal(RELLOW_LED_PIN_NAME, LED_DARK);
        }
        else
        {
            printf("error index");
        }
    }
do_cleanup:
    printf("do_cleanup...\r\n");
}

static void WifiConnectDemo(void)
{
    osThreadAttr_t attr;
    GpioInit();

    IoSetFunc(RED_LED_PIN_NAME, RED_LED_PIN_FUNCTION);
    IoSetFunc(GREEN_LED_PIN_NAME, GREEN_LED_PIN_FUNCTION);
    IoSetFunc(RELLOW_LED_PIN_NAME, RELLOW_LED_PIN_FUNCTION);

    GpioSetDir(RED_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(GREEN_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(RELLOW_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL)
    {
        printf("[WifiConnectDemo] Falied to create WifiConnectTask!\n");
    }
}
SYS_RUN(WifiConnectDemo);
// APP_FEATURE_INIT(WifiConnectDemo);