/**
 * @file startInfo.h
 * @brief 启动信息显示模块头文件
 * @author cepvor
 * @version 1.3
 * @date 2025/9/10
 * @license MIT License
 *
 * @attention
 * 本文件为启动信息显示模块头文件，包含如下内容：
 * - 显示启动信息函数声明
 */

#ifndef LIGHTPROJECT_SYSTEMSETUP_H
#define LIGHTPROJECT_SYSTEMSETUP_H

#include <Arduino.h>
#include "oled.h"
#include "font.h"

void showBootInfo();

#endif //LIGHTPROJECT_SYSTEMSETUP_H