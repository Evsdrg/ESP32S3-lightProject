/**
 * @file mqttConfig.h
 * @brief MQTT配置模块头文件
 * @author cepvor
 * @version 1.4
 * @date 2025/9/4
 * @license MIT License
 *
 * @attention
 * 本文件为MQTT配置模块头文件，包含如下内容：
 * - MQTT服务器地址、端口、用户名、密码等配置宏定义
 * - 设备ID及相关主题宏定义
 * - 全局变量声明与函数声明
 *
 * @note
 * 在使用前请根据实际MQTT服务器配置修改相关宏定义
 * 注意事项：
 * - 确保设备ID唯一，避免与其他设备冲突
 * - 根据需要调整缓存大小和连接超时时间
 */

#ifndef PLANTFORM_CLIONTEST_MQTTCONFIG_H
#define PLANTFORM_CLIONTEST_MQTTCONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_task_wdt.h"


#define MQTT_BROKER_ADDR "192.168.1.111"    // 服务器地址
#define MQTT_BROKER_PORT 1883               // 服务端口号
#define MQTT_USERNAME "123"                 // 账号（开发环境非必须）
#define MQTT_PASSWORD "456"                 // 密码（开发环境非必须）
#define MQTT_CLIENT_BUFF_SIZE 4096          // 客户端缓存大小（非必须）
#define MQTT_CLIENT_ID "esp32_client"       // 客户端ID
#define MQTT_CONNECT_TIMEOUT 5000           // MQTT连接超时时间(毫秒)
#define MQTT_MAX_RETRY_COUNT 3              // 最大重试次数
#define DEVICE_PREFIX "LIGHT_"                  // 设备ID前缀
#define DEVICE_NUMBER "4"                       // 设备编号（根据实际设备修改）
#define DEVICE_ID DEVICE_PREFIX DEVICE_NUMBER   // 设备ID（唯一标识符）
#define MQTT_TOPIC_DATA "device/" DEVICE_ID "/data"             // 数据上报主题
#define MQTT_TOPIC_HEARTBEAT "device/" DEVICE_ID "/heartbeat"   // 心跳包主题
#define MQTT_TOPIC_CONTROL "device/" DEVICE_ID "/control"       // 控制命令订阅主题


extern WiFiClient tcpClient;        // PubSubClient 实例，负责MQTT协议通信
extern PubSubClient mqttClient;     // 底层TCP传输客户端，由WiFiClient实现

void mqtt_callback(char *topic, byte *payload, unsigned int length);
void mqttConfig();
void mqttConnect();
void mqttSendData();
void mqttSendHeartbeat();

#endif //PLANTFORM_CLIONTEST_MQTTCONFIG_H