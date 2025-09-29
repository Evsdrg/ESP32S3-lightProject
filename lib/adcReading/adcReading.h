/**
 * @file timerManager.h
 * @brief ADC读取模块头文件
 * @author cepvor
 * @version 1.0
 * @date 2025/9/25
 * @license MIT License
 *
 * @attention
 * 本文件为ADC读取模块头文件，包含如下内容：
 * - 初始化和读取电压的函数声明
 * - 电压变量声明
 * - ADC引脚定义
 *
 * @note
 * 具体引脚定义请根据实际硬件选择
 * - 立创开发板：JLC_ADC_SOLAR_PIN (GPIO8), JCL_ADC_BATTERY_PIN (GPIO9)
 * - 自制核心板：ADC_SOLAR_PIN (GPIO7), ADC_BATTERY_PIN (GPIO10)
 */

#ifndef LIGHTPROJECT_ADCREADING_H
#define LIGHTPROJECT_ADCREADING_H

#include <Arduino.h>


#define JLC_ADC_SOLAR_PIN 8
#define JCL_ADC_BATTERY_PIN 9
#define ADC_SOLAR_PIN 7
#define ADC_BATTERY_PIN 10

extern int battery_mV;
extern int solar_mV;
extern int battery_percentage;      // 电池剩余电量百分比 (0-100)

void adcReadingInit();
void getVoltage();
int calculateBatteryPercentage(int voltage_mV);  // 计算电池剩余电量百分比


#endif //LIGHTPROJECT_ADCREADING_H