
#include <stdio.h>
#include <string.h>
#include <unistd.h>
 
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"
 
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
 
#include "wifi_connect.h"
 
int WIFI_STATE = 0;
 
 
// static char* SecurityTypeName(WifiSecurityType type)
// {
//     switch (type)
//     {
//     case WIFI_SEC_TYPE_OPEN:
//         return "OPEN";
//     case WIFI_SEC_TYPE_WEP:
//         return "WEP";
//     case WIFI_SEC_TYPE_PSK:
//         return "PSK";
//     case WIFI_SEC_TYPE_SAE:
//         return "SAE";
//     default:
//         break;
//     }
//     return "unkow";
// }
 
static void PrintLinkedInfo(WifiLinkedInfo* info)
{
    if (!info) return;
 
    static char macAddress[32] = {0};
    unsigned char* mac = info->bssid;
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("bssid: %s, rssi: %d, connState: %d, reason: %d, ssid: %s\r\n",
        macAddress, info->rssi, info->connState, info->disconnectedReason, info->ssid);
}
 
static int g_connected = 0;
static void OnWifiConnectionChanged(int state, WifiLinkedInfo* info)
{
    if (!info) return;
 
    printf("%s %d, state = %d, info = \r\n", __FUNCTION__, __LINE__, state);
    PrintLinkedInfo(info);
 
    if (state == WIFI_STATE_AVALIABLE) {
        g_connected = 1;
    } else {
        g_connected = 0;
    }
}
 
static void OnWifiScanStateChanged(int state, int size)
{
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
}
 
static void Delay(uint32_t ms)
{
    uint32_t usPerTicks = (1000*1000) / osKernelGetTickFreq();
    // printf("usPerTicks: %d\r\n", usPerTicks);
    osDelay((ms * 1000) / usPerTicks);
    usleep((ms * 1000) % usPerTicks);
}
 
static void WifiConnectTask(void *arg)
{
    (void)arg;
    WifiErrorCode errCode;
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = OnWifiConnectionChanged,
        .OnWifiScanStateChanged = OnWifiScanStateChanged
    };
    WifiDeviceConfig apConfig = {};
    int netId = -1;
 
    Delay(10);
    errCode = RegisterWifiEvent(&eventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);
 
    strcpy(apConfig.ssid, "BOARD");
    strcpy(apConfig.preSharedKey, "BOARD123456a?");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;
 
    while (1) {
        errCode = EnableWifi();
        printf("EnableWifi: %d\r\n", errCode);
        Delay(100);
 
        errCode = AddDeviceConfig(&apConfig, &netId);
        printf("AddDeviceConfig: %d\r\n", errCode);
 
        g_connected = 0;
        errCode = ConnectTo(netId);
        printf("ConnectTo(%d): %d\r\n", netId, errCode);
 
        while (!g_connected) {
            Delay(10);
        }
        printf("g_connected: %d\r\n", g_connected);
        Delay(3000);
 
        // 联网业务开始
        // 这里是网络业务代码...
        struct netif* iface = netifapi_netif_find("wlan0");
        if (iface) {
            err_t ret = netifapi_dhcp_start(iface);
            printf("netifapi_dhcp_start: %d\r\n", ret);
 
            Delay(2000); // wait DHCP server give me IP
            ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
            printf("netifapi_netif_common: %d\r\n", ret);
        }
        WIFI_STATE = 1;
 
        break;
        // 联网业务结束
 
        Disconnect(); // disconnect with your AP
 
        RemoveDevice(netId); // remove AP config
 
        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        Delay(500);
    }
}
 
void WifiConnect(void){
    osThreadAttr_t attr;
 
    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;
 
    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL) {
        printf("[WifiConnect] Falied to create WifiConnectTask!\n");
    }
}

// APP_FEATURE_INIT(WifiConnect);