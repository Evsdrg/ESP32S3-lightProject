/**
 * @file mqttConfig.cpp
 * @brief MQTT客户端配置与管理
 * @author cepvor
 * @version 1.2
 * @date 2025-09-02
 * @license MIT License
 *
 * @attention
 * 此文件主要用于MQTT客户端配置、连接管理、消息收发与回调处理
 * 相关依赖：
 *  - PubSubClient
 *  - ArduinoJson
 *  - WiFiClient
 * @note
 * 注意事项：
 *  - 相关参数请在mqttConfig.h中进行配置
 *  - 确保mqtt服务器中已经创建了对应的主题
 */

#include "mqttConfig.h"
#include "adcReading.h"
#include "taskCreate.h"
#include "brightnessConfig.h"
#include "getPM2dot5.h"
#include <FastLED.h>
#include <BH1750.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>

// MQTT相关配置信息
const char *mqttBrokerAddr = MQTT_BROKER_ADDR;              // 服务器地址
constexpr uint16_t mqttBrokerPort = MQTT_BROKER_PORT;           // 服务端口号
const char *mqttUsername = MQTT_USERNAME;                   // 账号（开发环境非必须）
const char *mqttPassword = MQTT_PASSWORD;                   // 密码（开发环境非必须）
constexpr uint16_t mqttClientBuffSize = MQTT_CLIENT_BUFF_SIZE;  // 客户端缓存大小（非必须）
String mqttClientId = MQTT_CLIENT_ID;                       // 客户端ID
const char *mqttTopicData = MQTT_TOPIC_DATA;                // 数据上报主题
const char *mqttTopicHeartbeat = MQTT_TOPIC_HEARTBEAT;      // 心跳包主题
const char *mqttTopicControl = MQTT_TOPIC_CONTROL;          // 控制命令订阅主题

PubSubClient mqttClient;        // PubSubClient 实例，负责MQTT协议通信
WiFiClient tcpClient;           // 底层TCP传输客户端，由WiFiClient实现

static uint8_t retryCount = 0;                  // 重试计数器，用于限制连续失败次数
static unsigned long lastConnectAttempt = 0;    // 上次尝试连接的时间戳（毫秒），用于控制重试间隔

/*
 * MQTT消息回调函数 —— 当订阅的主题收到消息时被自动调用
 * @param topic: 收到消息的主题名
 * @param payload: 消息负载（字节数组）
 * @param length: 负载长度（字节数）
 */
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    esp_task_wdt_reset();   // 重置ESP32任务看门狗，防止因处理耗时被复位
    Serial.println("\n---------------BEGIN---------------");
    Serial.printf("Message arrived in topic %s, length %d\n", topic, length);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print(static_cast<char>(payload[i]));     // 逐字节打印payload内容
    }
    Serial.println("\n----------------END----------------");

    /* ========== 消息处理逻辑 ========== */
    if (strcmp(topic, mqttTopicControl) == 0) {     // 判断是否为控制命令主题
        JsonDocument doc;                           // 创建JSON文档对象
        DeserializationError error = deserializeJson(doc, payload, length); // 反序列化JSON
        if (error) {
            Serial.println("JSON解析失败");
            return;
        }
        String command = doc["command"];            // 提取命令字段
        if (command == "set_brightness") {          // 处理“设置亮度”命令
            int newBrightness = doc["brightness"];  // 0~100百分比
            if (newBrightness >= 0 && newBrightness <= 100) {       // 百分比值 0~100
                brightness = map(newBrightness, 0, 100, 0, 255);    // 映射到PWM范围 0~255
                isAuto = false;                     // 手动模式
                Serial.printf("设置亮度为: %d\n", newBrightness);
            }
        }
        else if (command == "set_auto_mode") {      // 处理“设置自动模式”命令
            bool newAuto = doc["auto_mode"];        // true/false
            isAuto = newAuto;
            Serial.printf("设置自动模式: %d\n", isAuto);
        }
        esp_task_wdt_reset();                       // 处理完命令后再次喂狗
    }
    /* 可扩展其他主题的处理逻辑 */
}

/*
 * MQTT客户端初始化函数
 * 设置传输层、服务器地址、缓冲区大小及回调函数
 */
void mqttConfig() {
    mqttClient.setClient(tcpClient);                        // 绑定底层TCP客户端
    mqttClient.setServer(mqttBrokerAddr, mqttBrokerPort);   // 设置Broker地址和端口
    mqttClient.setBufferSize(mqttClientBuffSize);           // 设置内部缓冲区大小（影响最大消息长度）
    mqttClient.setCallback(mqtt_callback);                  // 注册消息回调函数
}

/*
 * MQTT连接函数 —— 尝试连接到MQTT Broker
 * 包含重试机制、错误处理、连接状态管理
 */
void mqttConnect() {
    if (WiFi.status() != WL_CONNECTED) {    // 检查WiFi连接
        Serial.println("WiFi未连接，跳过MQTT连接");
        return;
    }

    if (mqttClient.connected()) {           // 如果已连接，无需重复连接
        return;
    }

    unsigned long now = millis();
    if (now - lastConnectAttempt < 5000) {  // 控制重试频率：至少间隔5秒
        return;
    }

    if (retryCount >= MQTT_MAX_RETRY_COUNT) {   // 达到最大重试次数后暂停重试（防止频繁无效连接）
        Serial.println("MQTT连接重试次数达到上限，暂停重试");
        static unsigned long resetTime = 0;
        if (now - resetTime > 60000) {          // 1分钟后自动重置计数器，允许新一轮重试
            retryCount = 0;
            resetTime = now;
        }
        return;
    }

    lastConnectAttempt = now;       // 更新状态，准备连接
    retryCount++;

    Serial.printf("尝试连接MQTT服务器 (第%d次)...\n", retryCount);
    esp_task_wdt_reset();           // 喂看门狗，防止连接过程超时复位

    tcpClient.setTimeout(MQTT_CONNECT_TIMEOUT / 1000);      // 设置TCP连接超时时间（单位：秒）

    String clientId = String(MQTT_CLIENT_ID) + "_" + String(WiFi.macAddress());     // 构造唯一客户端ID（避免冲突）：基础ID + MAC地址（去除冒号）
    clientId.replace(":", "");                      // 去除MAC地址中的冒号

    if (mqttClient.connect(clientId.c_str())) {     // 尝试连接（无用户名密码版本，如需认证可使用带参版本）
        retryCount = 0;                             // 连接成功，重置重试计数
        esp_task_wdt_reset();                       // 喂看门狗，防止失败处理过程超时
        mqttClient.subscribe(mqttTopicControl);     // 订阅控制命令主题
        Serial.println("MQTT连接成功");
    }
    else {
        int state = mqttClient.state();                 // 获取失败原因
        Serial.printf("MQTT连接失败，错误代码: %d\n", state);
        esp_task_wdt_reset();                           // 喂看门狗，防止失败处理过程超时
        if (state == MQTT_CONNECT_BAD_CREDENTIALS ||    // 对于认证类错误，直接停止重试（避免无意义循环）
            state == MQTT_CONNECT_UNAUTHORIZED) {       // 两种报错均为认证失败
            Serial.println("认证错误，停止重试");
            retryCount = MQTT_MAX_RETRY_COUNT;          // 停止重试
        }
        /* 其他错误（如网络不通）允许继续重试 */
    }
}

/*
 * 发送设备数据包（周期性上报传感器数据）
 * 数据格式：JSON
 */
void mqttSendData() {
    if (mqttClient.connected()) {       // 仅在已连接状态下发送
        JsonDocument doc;               // 创建JSON文档对象
        doc["ambient_light"] = lux;     // 环境亮度
        uint8_t currentBrightness = isAuto ? brightnessAuto : brightness;   // 当前灯光亮度（0-255），根据模式选择
        uint8_t brightness100 = map(currentBrightness, 0, 255, 0, 100);     // 转换为百分比
        doc["light_brightness"] = brightness100;        // 灯光亮度百分比
        doc["temperature"] = temp.temperature;          // 环境温度
        doc["humidity"] = humidity.relative_humidity;   // 环境湿度
        doc["pm25"] = pm25_concentration;               // PM2.5传感器数值
        doc["battery_level"] = battery_percentage;      // 电池电量百分比
        doc["solar_voltage"] = solar_mV / 1000.0;       // 太阳能电压
        doc["auto_mode"] = isAuto;                      // 当前模式
        String payload;                                 // 序列化JSON为字符串
        serializeJson(doc, payload);             // 序列化JSON为字符串以便发布
        mqttClient.publish(mqttTopicData, payload.c_str());     // 发布到数据主题
    }
    mqttClient.loop();                  // 处理MQTT事务
    esp_task_wdt_reset();               // 喂看门狗，防止信息发送过程超时
    Serial.println("发送传感器数据成功");
}

/*
 * 发送心跳包（维持连接活跃、上报设备在线状态）
 */
void mqttSendHeartbeat() {
    if (mqttClient.connected()) {
        JsonDocument heartbeat;                 // 心跳包JSON对象
        heartbeat["device_id"] = DEVICE_ID;     // 设备唯一标识
        heartbeat["status"] = "online";         // 状态标记
        String payload;                         // 用于存储序列化后的JSON字符串
        serializeJson(heartbeat, payload);      // 序列化JSON为字符串以便发布
        mqttClient.publish(mqttTopicHeartbeat, payload.c_str());    // 发布到心跳主题
    }
    mqttClient.loop();          // 处理MQTT事务
    esp_task_wdt_reset();       // 喂看门狗，防止心跳包发送过程超时
    Serial.println("发送设备心跳包成功");
}

