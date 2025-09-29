/**
 * @file timerManager.h
 * @brief 定时器管理模块头文件
 * @author cepvor
 * @version 1.0
 * @date 2025/9/4
 * @license MIT License
 *
 * @attention
 * 本文件为定时器管理模块头文件，包含如下内容：
 * - 定时器初始化函数声明
 * - 定时器回调函数声明
 * - 定时标志变量声明
 */

#ifndef PLANTFORM_CLIONTEST_TIMERMANAGER_H
#define PLANTFORM_CLIONTEST_TIMERMANAGER_H

#include <Ticker.h>

extern volatile bool flag100ms, flag1s, flag5s, flag10s;

void timerInit();
void timer100msCallback();
void timer1sCallback();
void timer5sCallback();
void timer10sCallback();

#endif //PLANTFORM_CLIONTEST_TIMERMANAGER_H