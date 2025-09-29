/**
 * @file adcReading.cpp
 * @brief ADC读取电压值模块实现
 * @author cepvor
 * @version 1.0
 * @date 2025-09-25
 * @license MIT License
 *
 * @attention
 * 此文件主要用于ADC读取电压值，包括电池电压和太阳能电压
 * 通过位运算和整数化简化计算，提升运行效率
 *
 * @note
 * 注意事项：
 *  - 3300 是 ESP32 参考电压的毫伏值
 *  - L 后缀确保计算在 long（32位）整数中进行，防止 adcValue * 6600 溢出
 *  - 使用 >> 12 代替 / 4095 会引入约 0.024% 的系统误差，对于电池4.2V满充电压来说约为1mV
 *  - 对于太阳能6V满充电压来说约为1.47mV，这个误差在本应用中是可以接受的
 */

#include "adcReading.h"
#include "taskCreate.h"

int battery_mV = 0;   // 电池电压，单位：毫伏
int solar_mV = 0;     // 太阳能电压，单位：毫伏
int battery_percentage = 0;  // 电池剩余电量百分比 (0-100)

#ifdef isJLC
int adcSunPin = JLC_ADC_SOLAR_PIN;          // 立创开发板
int adcBatteryPin = JCL_ADC_BATTERY_PIN;    // 立创开发板
#else
int adcSunPin = ADC_SOLAR_PIN;              // 自制核心板
int adcBatteryPin = ADC_BATTERY_PIN;        // 自制核心板
#endif

void adcReadingInit() {
    analogReadResolution(12);
}

void getVoltage() {
    // --- 电池电压 (分压比 1/2) ---
    int batteryAdc = analogRead(adcBatteryPin);
    battery_mV = (batteryAdc * 6600L) >> 12; // ≈ (adc * 6600) / 4096


    // --- 太阳能电压 (分压比 33/133) ---
    int sunAdc = analogRead(adcSunPin);
#ifdef isJLC
    solar_mV = (sunAdc * 6600L) >> 12; // ≈ (adc * 6600) / 4096
#else
    solar_mV = (sunAdc * 13300L) >> 12; // ≈ (adc * 13300) / 4096
#endif

    // --- 计算电池剩余电量百分比 ---
    battery_percentage = calculateBatteryPercentage(battery_mV);
}

/**
 * 根据电池电压计算剩余电量百分比
 * 功能说明：基于锂电池的放电特性曲线，将电池电压转换为剩余电量百分比
 * 参数：voltage_mV - 电池电压值（毫伏）
 * 返回值：电池剩余电量百分比（0-100）
 *
 * 锂电池电压与电量对应关系：
 * - 4200mV (4.2V) = 100% 满电
 * - 4000mV (4.0V) = 85%  高电量
 * - 3800mV (3.8V) = 60%  中等电量
 * - 3700mV (3.7V) = 40%  较低电量
 * - 3600mV (3.6V) = 20%  低电量
 * - 3300mV (3.3V) = 5%   极低电量
 * - 3000mV (3.0V) = 0%   电池耗尽（保护电压）
 */
int calculateBatteryPercentage(int voltage_mV) {
    // 电池电压范围限制：3000mV(0%) 到 4200mV(100%)
    const int BATTERY_MIN_VOLTAGE = 3000;  // 电池保护电压，3.0V
    const int BATTERY_MAX_VOLTAGE = 4200;  // 电池满电电压，4.2V

    // 电压范围检查和限制
    if (voltage_mV <= BATTERY_MIN_VOLTAGE) {
        return 0;   // 电池耗尽或异常
    }
    if (voltage_mV >= BATTERY_MAX_VOLTAGE) {
        return 100; // 电池满电
    }

    /* 使用分段线性插值算法，模拟锂电池真实的放电曲线 */
    /* 锂电池的放电曲线并非线性，在不同电压区间有不同的斜率 */

    int percentage = 0;

    if (voltage_mV >= 4000) {
        // 高电压区间：4000mV-4200mV，对应85%-100%
        // 这个区间电压变化快，电量变化相对慢
        percentage = 85 + ((voltage_mV - 4000) * 15) / 200;
    }
    else if (voltage_mV >= 3800) {
        // 中高电压区间：3800mV-4000mV，对应60%-85%
        // 这是电池使用的主要区间，线性关系较好
        percentage = 60 + ((voltage_mV - 3800) * 25) / 200;
    }
    else if (voltage_mV >= 3700) {
        // 中电压区间：3700mV-3800mV，对应40%-60%
        // 电量下降开始加快
        percentage = 40 + ((voltage_mV - 3700) * 20) / 100;
    }
    else if (voltage_mV >= 3600) {
        // 低电压区间：3600mV-3700mV，对应20%-40%
        // 电量下降明显加快
        percentage = 20 + ((voltage_mV - 3600) * 20) / 100;
    }
    else if (voltage_mV >= 3300) {
        // 极低电压区间：3300mV-3600mV，对应5%-20%
        // 电池电量急剧下降，需要及时充电
        percentage = 5 + ((voltage_mV - 3300) * 15) / 300;
    }
    else {
        // 危险低电压区间：3000mV-3300mV，对应0%-5%
        // 电池即将耗尽，系统应该进入低功耗模式
        percentage = ((voltage_mV - 3000) * 5) / 300;
    }

    // 确保返回值在合理范围内
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    return percentage;
}
