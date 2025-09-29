/**
 * @file startInfo.cpp
 * @brief 启动信息显示模块实现
 * @author cepvor
 * @version 1.2
 * @date 2025-09-26
 * @license MIT License
 *
 * @attention
 * 此文件主要用于启动时候显示启动信息
 * 当在taskCreate.h中启用OLED显示时有效
 */

#include "startInfo.h"
#include "mqttConfig.h"
#include "taskCreate.h"

int i = 0;

/*
 * 显示启动信息
 */
void showBootInfo() {
#ifdef useOLED
    i++;
    char msg[50]={};
    OLED_NewFrame();
    OLED_PrintString(0, 0, "系统启动中", &font12x12, OLED_COLOR_NORMAL);
    sprintf(msg, "DevID:%s", DEVICE_NUMBER);
    OLED_PrintString(84, 0, msg, &font12x12, OLED_COLOR_NORMAL);
    switch (i) {
        case 1:
            OLED_PrintString(0, 16, "I2C总线设备初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 2:
            OLED_PrintString(0, 16, "I2C总线设备初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "亮度控制初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 3:
            OLED_PrintString(0, 16, "I2C总线设备初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "亮度控制初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "空气检测初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 4:
            OLED_PrintString(0, 16, "亮度控制初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "空气检测初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "看门狗初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 52, "正在连接网络", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 5:
            OLED_PrintString(0, 16, "空气检测初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "看门狗初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "网络连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 52, "正在连接MQTT服务器", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 6:
            OLED_PrintString(0, 16, "空气检测初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "看门狗初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "网络连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 52, "MQTT服务器连接成功", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 7:
            OLED_PrintString(0, 16, "看门狗初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "网络连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "MQTT服务器连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 52, "WS2812初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            break;
        case 8:
            OLED_PrintString(0, 16, "网络连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 28, "MQTT服务器连接成功", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 40, "WS2812初始化完毕", &font12x12, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 52, "任务创建完毕", &font12x12, OLED_COLOR_NORMAL);
            break;
        default:
            break;
    }
    OLED_ShowFrame();
    delay(80);

#else
    Serial.println("未启用 OLED 显示");
    delay(20);
#endif
}
