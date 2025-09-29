/**
 * @file taskCreate.cpp
 * @brief FreeRTOS 任务创建与管理
 * @author cepvor
 * @version 1.8
 * @date 2025-09-04
 * @license MIT License
 *
 * @attention
 * 此文件主要用于FreeRTOS 任务创建与管理，分配任务至双核，协调传感器、MQTT、灯光控制、串口输出等
 * 相关依赖：
 * - timerManager, mqttConfig, wifiConfig,
 * - PubSubClient, FastLED, Adafruit_AHTX0, BH1750
 *
 * @note
 * 注意事项：
 * - 创建的任务需要在taskCreate.h中声明
 * - 确保各任务函数已在相应模块中定义
 * - 根据实际需求调整任务优先级与栈大小
 */

#include "taskCreate.h"
#include "timerManager.h"
#include "mqttConfig.h"
#include "wifiConfig.h"
#include <PubSubClient.h>
#include "adcReading.h"
#include "oled.h"
#include "brightnessConfig.h"   // 添加亮度配置模块头文件
#include "getPM2dot5.h"         // 添加PM2.5模块头文件

/* ==================== 任务创建函数（Core 0） ==================== */
/*
 * 在 Core 0 上创建网络与串口相关任务
 * Core 0 通常用于处理 WiFi、MQTT、HTTP 等网络协议栈
 */
void taskCreateCore0() {
    /* 创建 MQTT 数据上报任务 */
    BaseType_t resultMqttData = xTaskCreatePinnedToCore(
        mqttDataTask,           // 任务函数
        "mqtt_Data_Task",       // 任务名称（字符串）
        8192,                   // 栈大小（字节），MQTT+JSON序列化需较大栈空间
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        1,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        0                       // 核心编号：0表示Core 0
    );
    /* 创建 MQTT 心跳包任务 */
    BaseType_t resultMqttHeart = xTaskCreatePinnedToCore(
        mqttHeartbeatTask,      // 任务函数
        "mqtt_Heartbeat_Task",  // 任务名称（字符串）
        8192,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        1,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        0                       // 核心编号：0表示Core 0
    );

    /* 错误检查 */
    if (resultMqttData != pdPASS) {
        Serial.println("mqttDataTask 创建失败");
    }
    if (resultMqttHeart != pdPASS) {
        Serial.println("mqttHeartbeatTask 创建失败");
    }
}

/* ==================== 任务创建函数（Core 1） ==================== */
/*
 * 在 Core 1 上创建传感器采集与灯光控制任务
 * Core 1 用于处理实时性要求较高的本地控制与传感器读取
 */
void taskCreateCore1() {
    /* 创建传感器采集任务 */
    BaseType_t resultI2C = xTaskCreatePinnedToCore(
        getI2CTask,             // 任务函数
        "I2C_Task",             // 任务名称（字符串）
        4096,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        4,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        1                       // 核心编号：1表示Core 1
    );
    /* 创建灯光控制任务 */
    BaseType_t resultLight = xTaskCreatePinnedToCore(
        lightSetTask,           // 任务函数
        "lightSet_Task",        // 任务名称（字符串）
        4096,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        5,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        1                       // 核心编号：1表示Core 1
    );
    /* 创建串口打印任务用于调试，每秒输出传感器数据 */
    BaseType_t resultSP = xTaskCreatePinnedToCore(
        serialPrintTask,        // 任务函数
        "serialPrint_Task",     // 任务名称（字符串）
        4096,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        2,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        1                       // 核心编号：1表示Core 1
    );

    /* 创建PM2.5数据处理任务 */
    BaseType_t resultPM25 = xTaskCreatePinnedToCore(
        pm25DataTask,           // 任务函数
        "PM25_Data_Task",       // 任务名称（字符串）
        2048,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        3,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        1                       // 核心编号：1表示Core 1
    );

#ifdef useOLED
    /* 创建 OLED 显示任务用于调试，每秒输出传感器数据 */
    BaseType_t resultOLEDPrt = xTaskCreatePinnedToCore(
        oledPrintTask,          // 任务函数
        "oledPrint_Task",       // 任务名称（字符串）
        4096,                   // 栈大小（字节）
        nullptr,                // 传递给任务的参数，如果不需要可以设为nullptr
        1,                      // 任务优先级（1-25，数字越大优先级越高）
        nullptr,                // 任务句柄，如果不需要可以设为nullptr
        1                       // 核心编号：1表示Core 1
    );
#endif
    /* 错误检查 */
    if (resultI2C != pdPASS) {
        Serial.println("I2CTask创建失败");
    }
    if (resultLight != pdPASS) {
        Serial.println("lightSetTask创建失败");
    }
    if (resultSP != pdPASS) {
        Serial.println("SerialPrintTask 创建失败");
    }
    if (resultPM25 != pdPASS) {
        Serial.println("PM25DataTask 创建失败");
    }
#ifdef useOLED
    if (resultOLEDPrt != pdPASS) {
        Serial.println("oledPrintTask 创建失败");
    }
#endif
}

/* ==================== 任务函数实现 ==================== */

/*
 * ———————— 传感器采集任务 ————————
 * 周期性读取 AHT20 温湿度传感器与 BH1750 光照传感器
 * 数据存入全局变量供其他任务使用
 */
sensors_event_t humidity, temp; // 温湿度事件结构体（来自Adafruit_Sensor）
Adafruit_AHTX0 aht;             // AHT20 温湿度传感器对象
BH1750 lightMeter;              // BH1750 光照强度传感器对象
float lux = 500.0;              // 环境光照强度（单位：lux），默认初始值

void getI2CTask(void *pvParameters) {
    (void) pvParameters;         // 不进行传参则固定使用此代码
    while (true) {
        aht.getEvent(&humidity, &temp);     // 读取温湿度
        lux = lightMeter.readLightLevel();  // 读取光照强度（lux）
        getVoltage();                       // 读取电池与太阳能电压
        vTaskDelay(DELAY_100MS);            // 任务运行周期（100ms）
    }
}

/*
 * ———————— WiFi 连接检测任务 ————————
 * 监听 WiFi 是否断开连接
 * 若断线则尝试重连（每10秒）
 */
TaskHandle_t xReconnectHandle = nullptr;

void wifiReconnectTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    // 永久等通知（掉线时收到）
        Serial.println("[WiFi] 15 秒后开始重连 ...");
        vTaskDelay(DELAY_10S);      // 等待10秒再重连，避免频繁尝试
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   // 真正重连
    }
}

/*
 * ———————— MQTT 数据上报任务 ————————
 * 周期性连接MQTT并上报传感器数据（每10秒）
 * 自动处理重连逻辑
 */
void mqttDataTask(void *pvParameters) {
    (void) pvParameters;
    esp_task_wdt_add(nullptr);     // 将当前任务注册到ESP32任务看门狗（防止长时间阻塞导致复位）
    while (true) {
        mqttConnect();          // 尝试连接（含重试机制）
        mqttSendData();         // 发送数据（仅在已连接时有效）
        vTaskDelay(DELAY_10S);  // 任务运行周期（10s）
    }
}

/*
 * ———————— MQTT 心跳包任务 ————————
 * 周期性发送心跳包，维持连接活跃（每5秒）
 */
void mqttHeartbeatTask(void *pvParameters) {
    (void) pvParameters;
    esp_task_wdt_add(nullptr);     // 注册看门狗
    while (true) {
        mqttConnect();          // 尝试连接（含重试机制）
        mqttSendHeartbeat();    // 发送心跳（仅在已连接时有效）
        vTaskDelay(DELAY_5S);   // 任务运行周期（5s）
    }
}


/*
 * ———————— 灯光控制任务 ————————
 * 使用新的亮度控制模块，支持运动检测和平滑变化曲线
 * 每50ms刷新一次，保证灯光无闪烁并匹配变化曲线
 */
uint8_t ledCount = LED_COUNT;   // LED灯珠数量
CRGB leds[LED_COUNT];           // FastLED 像素缓冲区
bool isAuto = true;             // 是否启用自动亮度模式
uint8_t brightnessAuto = 0, brightness = 0; // 各模式亮度值

void lightSetTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        if (isAuto == true) {   // 自动模式：使用新的亮度控制算法
            brightnessAuto = calculatePerceivedBrightness(lux);
            fill_solid(leds, ledCount, CRGB(brightnessAuto, brightnessAuto, brightnessAuto));   // 设置所有LED为同一灰度值（白光）
        }
        else {      // 手动模式：使用设定值
            fill_solid(leds, ledCount, CRGB(brightness, brightness, brightness));
        }
        FastLED.show();         // 刷新LED
        vTaskDelay(DELAY_50MS); // 任务运行周期改为50ms，匹配亮度变化曲线
    }
}

/*
 * ———————— 串口打印调试任务 ————————
 * 每秒打印一次传感器数据与灯光状态，用于调试和监控
 */
void serialPrintTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {      // 输出传感器信息与设定值
        Serial.print("Light: ");
        Serial.print(lux);
        Serial.print("lx ");
        Serial.print(", brightnessAuto: ");
        Serial.print(brightnessAuto);
        Serial.print(", brightness: ");
        Serial.print(brightness);
        Serial.print(", isMove: ");
        Serial.print(isMove ? "YES" : "NO");
        Serial.print(", baseBrightness: ");
        Serial.println(baseBrightness);
        Serial.print("Temperature: ");
        Serial.print(temp.temperature);
        Serial.print(" ℃");
        Serial.print("， Humidity: ");
        Serial.print(humidity.relative_humidity);
        Serial.print("% rH");
        Serial.print("， PM2.5: ");
        Serial.print(pm25_concentration);
        Serial.println(" µg/m³");
        vTaskDelay(DELAY_1S);   // 任务运行周期（1s）
    }
}

/*
 * ———————— PM2.5数据处理任务 ————————
 * 高频处理串口接收的PM2.5数据，每10ms检查一次
 * 确保不丢失任何数据包
 */
void pm25DataTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        pm25_update();  // 处理串口接收的数据
        vTaskDelay(DELAY_100MS);  // 每10ms检查一次，确保不丢失数据
    }
}

#ifdef useOLED
/*
 * ———————— 屏幕显示任务 ————————
 * 每秒刷新一次屏幕显示内容，用于调试和监控
 */
void oledPrintTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        char msg[50] = {};  // 用于存储格式化字符串
        OLED_NewFrame();
        OLED_DrawRectangle(0, 17, 127, 46, OLED_COLOR_NORMAL);
        sprintf(msg, "WiFi:%s", WiFi.status() == WL_CONNECTED ? "V" : "X");
        OLED_PrintString(0, 0, msg, &font12x12, OLED_COLOR_NORMAL);
        sprintf(msg, "MQTT:%s", mqttClient.connected() ? "V" : "X");
        OLED_PrintString(42, 0, msg, &font12x12, OLED_COLOR_NORMAL);
        sprintf(msg, "DevID:%s", DEVICE_NUMBER);
        OLED_PrintString(84, 0, msg, &font12x12, OLED_COLOR_NORMAL);

        sprintf(msg, "IP:%s", WiFi.localIP().toString().c_str());
        OLED_PrintString(2, 18, msg, &font12x12, OLED_COLOR_NORMAL);

        sprintf(msg, "Lux:%.1f", lux);
        OLED_PrintString(2, 28, msg, &font12x12, OLED_COLOR_NORMAL);
        sprintf(msg, "Light:%d", isAuto ? brightnessAuto : brightness);
        OLED_PrintString(70, 28, msg, &font12x12, OLED_COLOR_NORMAL);

        OLED_DrawImage(2,39,&temperatureImg,OLED_COLOR_NORMAL);
        sprintf(msg, "%.1f", temp.temperature);
        OLED_PrintString(15, 40, msg, &font12x12, OLED_COLOR_NORMAL);
        OLED_DrawImage(44,39,&humidityImg,OLED_COLOR_NORMAL);
        sprintf(msg, "%.1f", humidity.relative_humidity);
        OLED_PrintString(57, 40, msg, &font12x12, OLED_COLOR_NORMAL);
        OLED_DrawImage(85,39,&PM2dot5Img,OLED_COLOR_NORMAL);
        sprintf(msg, "%d", pm25_concentration);
        OLED_PrintString(99, 40, msg, &font12x12, OLED_COLOR_NORMAL);

        OLED_DrawImage(3,51,&batteryImg,OLED_COLOR_NORMAL);
        sprintf(msg, "%d%%", battery_percentage);
        OLED_PrintString(18, 51, msg, &font12x12, OLED_COLOR_NORMAL);
        OLED_DrawImage(70,51,&solarImg,OLED_COLOR_NORMAL);
        sprintf(msg, "%dmV", solar_mV);
        OLED_PrintString(86, 51, msg, &font12x12, OLED_COLOR_NORMAL);


        OLED_ShowFrame();
        vTaskDelay(DELAY_500MS);   // 任务运行周期（1s）
    }
}
#endif
