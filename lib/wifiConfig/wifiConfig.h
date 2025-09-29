/**
 * @file wifiConfig.h
 * @brief wifi相关功能头文件
 * @author cepvor
 * @version 1.0
 * @date 2025/9/2
 * @license MIT License
 *
 * @attention
 * 本文件为WiFi相关功能头文件，包含如下内容：
 * - WiFi名称与密码宏定义
 * - WiFi初始化与连接函数声明
 */

#ifndef PLANTFORM_CLIONTEST_WIFICONFIG_H
#define PLANTFORM_CLIONTEST_WIFICONFIG_H
#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID ""       // WiFi名称
#define WIFI_PASSWORD ""   // WiFi密码


void wifiConfig();

#endif //PLANTFORM_CLIONTEST_WIFICONFIG_H
