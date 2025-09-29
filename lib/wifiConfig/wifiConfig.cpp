/**
 * @file wifiConfig.cpp
 * @brief WiFi 连接配置与管理
 * @author cepvor
 * @version 1.2
 * @date 2025-09-02
 * @license MIT License
 *
 * @attention
 * 此文件主要用于WiFI客户端配置、连接管理与回调处理
 * 依赖库：WiFi
 *
 * @note
 * 注意事项：
 * - 相关参数请在wifiConfig.h中进行配置
 * - 只有在WiFi连接成功后才能进行MQTT连接
 */


#include "wifiConfig.h"

#define MAX_RETRY 20          // 最大重试次数

const char* ssid = WIFI_SSID;           // WiFi名称
const char* password = WIFI_PASSWORD;   // WiFi密码
extern TaskHandle_t xReconnectHandle;   // WiFi重连任务句柄

/*
 * 初始化并连接 WiFi 网络
 * 支持连接状态轮询
 * 输出详细调试信息
 */
void wifiConfig() {
    uint16_t retry = 0;

    Serial.println("\n***********************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);         // 尝试连接 
    while (WiFi.status() != WL_CONNECTED) {
        if (++retry > MAX_RETRY) {      // 超时
            Serial.println("\nWiFi connect timeout !");
            return;                     // 非阻塞方式
        }
        delay(500);
        Serial.print('.');
    }
    // 连接成功
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/*
 * 事件回调仅发送通知，防止阻塞
 */
void onWiFiEvent(arduino_event_id_t event) {
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:   // 连接断开
        xTaskNotifyGive(xReconnectHandle);      // 通知重连任务
        break;
    default:
        break;
    }
}

