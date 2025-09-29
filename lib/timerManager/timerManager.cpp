/**
 * @file timerManager.cpp
 * @brief 定时器管理模块实现
 * @author cepvor
 * @version 1.1
 * @date 2025/9/4
 * @license MIT License
 *
 * @description
 * 本模块实现定时器管理功能，包括：
 * - 多个定时器的初始化与回调
 * - 定时器中断与主循环的通信
 * @note
 * 在本次工程中，一般任务不使用定时器，此文件作为备用
 */

#include "timerManager.h"


Ticker timer100ms, timer1s, timer5s, timer10s;

/* 中断与各循环通信 */
volatile bool flag100ms = true, flag1s = true, flag5s = true, flag10s = false;

/* 各定时器初始化 */
void timerInit() {
    timer100ms.attach_ms(100, timer100msCallback);
    timer1s.attach(1.0, timer1sCallback);
    timer5s.attach(5.0, timer5sCallback);
    timer10s.attach(10.0, timer10sCallback);
}

/* 100ms定时器回调 */
void timer100msCallback() {
    flag100ms = true;
}

/* 1s定时器回调 */
void timer1sCallback() {
    flag1s = true;
}

/* 5s定时器回调 */
void timer5sCallback() {
    flag5s = true;
}

/* 10s定时器回调 */
void timer10sCallback() {
    flag10s = true;
}

