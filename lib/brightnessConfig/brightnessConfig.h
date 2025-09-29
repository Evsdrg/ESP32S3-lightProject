/**
 * @file brightnessConfig.h
 * @brief 亮度控制模块头文件
 * @author cepvor
 * @version 2.2
 * @date 2025/9/2
 * @license MIT License
 *
 * @attention
 * 本文件为亮度控制模块头文件，包含如下内容：
 * - 亮度阈值和对应亮度档位的宏定义
 * - 亮度变化曲线参数的宏定义
 * - 运动检测和手动触发引脚的宏定义
 * - 全局变量声明与函数声明
 *
 * @note
 * 具体引脚定义请根据实际硬件选择
 * 立创开发板：JLC_MOTION_PIN (GPIO16), JLC_KEY1_PIN (GPIO3), JLC_KEY2_PIN (GPIO4)
 * 自制核心板：MOTION_PIN (GPIO15), KEY1_PIN (GPIO1), KEY2_PIN (GPIO2)
 */

#ifndef BRIGHTNESSCONFIG_H
#define BRIGHTNESSCONFIG_H

#include <Arduino.h>

/* 亮度阈值定义 */
#define LUX_THRESHOLD_HIGH 500      // 500lux阈值
#define LUX_THRESHOLD_MID 300       // 300lux阈值
#define LUX_THRESHOLD_LOW 100       // 100lux阈值

/* 对应亮度档位 */
#define BRIGHTNESS_HIGH_LUX 50      // 500lux时的亮度
#define BRIGHTNESS_MID_LUX 80       // 300lux时的亮度
#define BRIGHTNESS_LOW_LUX 110      // 100lux时的亮度
#define BRIGHTNESS_MAX 255          // 运动检测时的最高亮度

/* 变化曲线参数 (50ms周期) */
#define BRIGHTNESS_UP_STEPS 40      // 2秒上升 (2000ms / 50ms = 40步)
#define BRIGHTNESS_DOWN_STEPS 60    // 3秒下降 (3000ms / 50ms = 60步)

/* 运动检测引脚与手动触发引脚 */
// 立创开发板引脚定义
#define JLC_MOTION_PIN 16
#define JLC_KEY1_PIN 3
#define JLC_KEY2_PIN 4
// 自制核心板引脚定义
#define MOTION_PIN 15
#define KEY1_PIN 1
#define KEY2_PIN 2


/* 全局变量声明 */
extern volatile bool isMove;        // 运动检测标志
extern uint8_t currentBrightness;   // 当前实际亮度值
extern uint8_t targetBrightness;    // 目标亮度值
extern uint8_t baseBrightness;      // 基础亮度值（根据环境光计算）

/* 函数声明 */
void brightnessInit();              // 初始化亮度控制模块
void motionISR();                   // 运动检测中断服务函数
void key1ISR();                     // KEY1中断服务函数 - 手动触发运动检测
void key2ISR();                     // KEY2中断服务函数 - 手动消除运动检测
uint8_t calculateBaseBrightness(float Lux);  // 根据环境光计算基础亮度
uint8_t updateBrightness();         // 更新亮度值，返回当前亮度
uint8_t calculatePerceivedBrightness(float Lux); // 总体亮度计算函数

#endif //BRIGHTNESSCONFIG_H