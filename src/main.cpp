/**
 * @file main.cpp
 * @brief ESP32-S3 智慧路灯
 * @author cepvor
 * @version 1.6
 * @date 2025-09-10
 * @license MIT License
 *
 * @attention
 * 本项目基于ESP32-S3开发板，实现了如下功能：
 * - 集成AHT20温湿度传感器、BH1750光照强度传感器、WS2812 LED灯板和OLED显示屏等设备
 * - 通过WiFi连接到MQTT服务器，实现远程控制和数据上报功能
 * 开发环境为Arduino模式的PlatformIO，使用FreeRTOS进行任务调度
 *
 * @note
 * 注意事项：
 * - 需要使用的库必须在main.cpp中被包含，否则会编译出错
 * - 确保所有库符合platformio.ini中设定的版本，以避免兼容性问题
 * - 请根据实际硬件连接，在对应模块文件中调整引脚定义
 * - 需要创建新任务的时候，请在taskCreate.cpp中添加
 * - 烧录速率根据需求自行调整，若需要更快烧录请使用原生USB接口，并在platformio.ini中设置较高的速度
 */

#include <Arduino.h>
#include "startInfo.h"
#include <Wire.h>
#include <FastLED.h>
#include <Adafruit_AHTX0.h>
#include <BH1750.h>
#include <HardwareSerial.h>
#include "wifiConfig.h"
#include "mqttConfig.h"
#include "timerManager.h"
#include "taskCreate.h"
#include "oled.h"
#include "font.h"
#include "adcReading.h"
#include "brightnessConfig.h"   // 添加亮度配置模块头文件
#include "getPM2dot5.h"         // 添加PM2.5模块头文件

#define timeout_seconds 20      // 超时时间（20s）
#define panic_on_timeout true   // 超时后是否触发panic

void setup() {
    Serial.begin(115200);       // 初始化串口速率
#ifdef isJLC
    Wire.begin(1, 2); // 初始化I2C总线 前者SDA 后者SCL, 立创开发板
#else
    Wire.begin(8, 9); // 初始化I2C总线 前者SDA 后者SCL, 自制核心板
#endif
    Wire.setClock(400000);      // 设置I2C时钟频率为400kHz

    Serial.println("初始化I2C总线设备");
    lightMeter.begin();         // BH1750光照强度传感器初始化
    aht.begin();                // AHT20温湿度传感器初始化
#ifdef useOLED
    OLED_Init();                // OLED显示屏初始化
#endif
    adcReadingInit();           // ADC初始化
    showBootInfo();             // 显示启动信息1

    Serial.println("初始化亮度控制模块");
    brightnessInit();           // 初始化亮度控制模块（包含运动检测中断）
    showBootInfo();             // 显示启动信息2

    Serial.println("初始化PM2.5传感器");
    pm25_init();                // 初始化PM2.5串口通信
    showBootInfo();             // 显示启动信息3

    Serial.println("初始化看门狗");
    esp_task_wdt_init(timeout_seconds, panic_on_timeout);
    showBootInfo();             // 显示启动信息4

    Serial.println("初始化网络连接");
    wifiConfig();               // 连接网络
    showBootInfo();             // 显示启动信息5
    mqttConfig();               // 设置MQTT
    showBootInfo();             // 显示启动信息6

    // Serial.println("设置各个定时器");
    // timerInit();

    Serial.println("初始化WS2812");
    CFastLED::addLeds<WS2812, ledPin, GRB>(leds, LED_COUNT);     // 初始化FastLED
    fill_solid(leds, LED_COUNT, CRGB(0, 0, 0));   // 全部清零（即设置亮度为0）
    FastLED.show();             // 更新显示
    showBootInfo();             // 显示启动信息7

    Serial.println("任务创建");
    taskCreateCore0();
    taskCreateCore1();
    showBootInfo();             // 显示启动信息8
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10));
}
