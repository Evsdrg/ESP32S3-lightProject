/**
 * @file taskCreate.h
 * @brief 任务创建模块头文件
 * @author cepvor
 * @version 2.3
 * @date 2025/9/4
 * @license MIT License
 *
 * @attention
 * 本文件为任务创建模块头文件，包含如下内容：
 * - 任务创建函数声明与任务函数声明
 * - 任务周期宏定义
 * - 相关外设对象和变量声明
 *
 * @note
 * 注意事项：
 * - 根据实际硬件选择是否使用立创开发板
 * - 根据需要选择是否启用OLED显示任务
 * - 任务周期可根据实际需求调整
 * - 新增任务记得声明任务函数和创建任务
 */

#ifndef PLANTFORM_CLIONTEST_TASKCREATE_H
#define PLANTFORM_CLIONTEST_TASKCREATE_H

#include <Adafruit_AHTX0.h>
#include <Arduino.h>
#include <BH1750.h>
#include <FastLED.h>
#include "brightnessConfig.h"

/* 任务执行周期 */
#define DELAY_10S    pdMS_TO_TICKS(10000)
#define DELAY_5S     pdMS_TO_TICKS(5000)
#define DELAY_1S     pdMS_TO_TICKS(1000)
#define DELAY_500MS  pdMS_TO_TICKS(500)
#define DELAY_100MS  pdMS_TO_TICKS(100)
#define DELAY_50MS   pdMS_TO_TICKS(50)
#define DELAY_20MS   pdMS_TO_TICKS(20)

/* 是否使用立创开发板 */
#define isJLC
/* 是否启用OLED屏幕 */
#define useOLED

/* 任务创建 */
void taskCreateCore0();

void taskCreateCore1();

/* I2C任务相关 */
extern sensors_event_t humidity, temp;  // 温湿度事件结构体（来自Adafruit_Sensor）
extern Adafruit_AHTX0 aht;              // AHT20 温湿度传感器对象
extern BH1750 lightMeter;               // BH1750 光照强度传感器对象
extern float lux;                       // 环境光照强度（单位：lux），默认初始值

void getI2CTask(void* pvParameters);

/* WiFi任务相关 */
extern TaskHandle_t xReconnectHandle;   // WiFi重连任务句柄

void wifiReconnectTask(void* pvParameters);

/* MQTT任务相关 */
void mqttDataTask(void* pvParameters);

void mqttHeartbeatTask(void* pvParameters);

/* 灯控任务相关 */
#define JLC_LED_PIN 38          // 使用立创开发板时候的LED数据引脚
#define LED_PIN 6               // LED数据引脚
#define LED_COUNT 16            // LED灯珠数量
extern uint8_t ledCount;        // LED灯珠数量
#ifdef isJLC
constexpr uint8_t ledPin = JLC_LED_PIN;   // 立创开发板使用的LED数据引脚
#else
constexpr uint8_t ledPin = LED_PIN;       // LED数据引脚
#endif
extern CRGB leds[LED_COUNT];    // FastLED 像素缓冲区
extern uint8_t brightness;      // 手动模式亮度值
extern uint8_t brightnessAuto;  // 自动模式亮度值
extern bool isAuto;             // 是否启用自动亮度模式
void lightSetTask(void* pvParameters);

/* 串口打印任务相关 */
void serialPrintTask(void* pvParameters);

/* PM2.5数据处理任务相关 */
void pm25DataTask(void* pvParameters);

#ifdef useOLED
/* OLED显示任务相关 */
void oledPrintTask(void* pvParameters);
#endif

#endif //PLANTFORM_CLIONTEST_TASKCREATE_H