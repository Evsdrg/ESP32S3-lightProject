/**
 * @file brightnessConfig.cpp
 * @brief 智能亮度控制模块实现
 * @author cepvor
 * @version 2.0
 * @date 2025/9/2
 * @license MIT License
 *
 * @description
 * 本模块实现智能LED亮度控制功能，包括：
 * - 基于环境光传感器的自适应亮度调节
 * - 运动检测触发的亮度增强
 * - 平滑的亮度变化曲线算法
 * - 支持手动调试按钮控制
 *
 * 主要特性：
 * - 三档环境光亮度控制（500/300/100 lux）
 * - 运动检测5秒超时机制
 * - 平滑的非线性亮度变化曲线
 * - 兼容立创开发板和自制核心板
 *
 * 硬件接口：
 * - motionPin: PIR运动检测传感器输入
 * - key1Pin: 手动触发运动检测按钮（内部上拉）
 * - key2Pin: 手动消除运动检测按钮（内部上拉）
 */

#include "brightnessConfig.h"
#include "taskCreate.h"
#include <Adafruit_AHTX0.h>
#include <BH1750.h>
#include <FastLED.h>

/* ========== 全局变量定义区域 ========== */
/* 这些变量用于存储系统的当前状态，在整个程序运行期间都会被使用 */

volatile bool isMove = false;           // 运动检测标志（volatile表示这个变量可能被中断函数修改，告诉编译器不要优化它）
uint8_t currentBrightness = 0;          // 当前实际亮度值（0-255，0是最暗，255是最亮）
uint8_t targetBrightness = 0;           // 目标亮度值（系统想要达到的亮度）
uint8_t baseBrightness = 0;             // 基础亮度值（根据环境光传感器计算出的基本亮度）

/* ========== 硬件引脚配置区域 ========== */
/* 根据不同的开发板型号，选择对应的引脚编号 */
#ifdef isJLC
/* 立创开发板的引脚定义 */
int motionPin = JLC_MOTION_PIN;         // 运动检测传感器连接的引脚
int key1Pin = JLC_KEY1_PIN;             // KEY1按钮连接的引脚（用于手动触发运动检测）
int key2Pin = JLC_KEY2_PIN;             // KEY2按钮连接的引脚（用于手动取消运动检测）
#else
/* 自制核心板的引脚定义 */
int motionPin = MOTION_PIN;             // 运动检测传感器连接的引脚
int key1Pin = KEY1_PIN;                 // KEY1按钮连接的引脚
int key2Pin = KEY2_PIN;                 // KEY2按钮连接的引脚
#endif

/* ========== 私有变量定义区域 ========== */
/* 这些变量只在本文件内部使用，用于控制亮度变化的细节 */
static uint32_t lastMotionTime = 0;     // 上次检测到运动的时间（毫秒）
static uint32_t motionTimeout = 5000;   // 运动检测超时时间(5秒，即5000毫秒)
static int16_t brightnessStep = 0;      // 当前亮度变化已执行的步数（用于控制变化速度）
static bool isRising = false;           // 标记当前是否正在增加亮度
static bool isFalling = false;          // 标记当前是否正在降低亮度
static uint8_t startBrightness = 0;     // 记录亮度变化开始时的初始亮度值

/**
 * 初始化亮度控制模块
 * 功能说明：这个函数在系统启动时被调用，用于设置各种硬件接口和初始参数
 */
void brightnessInit() {
    /* ===== 设置引脚工作模式 ===== */
    pinMode(motionPin, INPUT_PULLDOWN);     // 设置运动检测引脚为输入模式，并启用内部下拉
    pinMode(key1Pin, INPUT_PULLUP);         // KEY1按钮，内部上拉，用于手动触发运动检测
    pinMode(key2Pin, INPUT_PULLUP);         // KEY2按钮，内部上拉，用于手动消除运动检测

    /* ===== 配置中断处理 ===== */
    /* 为运动检测引脚配置中断：当引脚电平从低变高时（RISING上升沿），调用motionISR函数 */
    attachInterrupt(digitalPinToInterrupt(motionPin), motionISR, RISING);
    /* 为KEY1按钮配置中断：当按钮被按下时（引脚电平从高变低，FALLING下降沿），调用key1ISR函数 */
    attachInterrupt(digitalPinToInterrupt(key1Pin), key1ISR, FALLING);
    /* 为KEY2按钮配置中断：当按钮被按下时，调用key2ISR函数 */
    attachInterrupt(digitalPinToInterrupt(key2Pin), key2ISR, FALLING);

    /* ===== 初始化所有变量为默认值 ===== */
    isMove = false;                     // 初始状态：没有检测到运动
    currentBrightness = 0;              // 初始亮度：0（灯是关闭的）
    targetBrightness = 0;               // 初始目标亮度：0
    baseBrightness = 0;                 // 初始基础亮度：0
    brightnessStep = 0;                 // 初始变化步数：0
    isRising = false;                   // 初始状态：不在增加亮度
    isFalling = false;                  // 初始状态：不在降低亮度

    /* 向串口输出初始化完成的信息（用于调试） */
    Serial.println("亮度控制模块初始化完成");
    Serial.println("IO15: 原有运动检测功能");
    Serial.println("KEY1(IO1): 手动触发运动检测");
    Serial.println("KEY2(IO2): 手动消除运动检测");
}

/**
 * 运动检测中断服务函数
 * 功能说明：当PIR运动检测传感器检测到运动时，这个函数会被自动调用
 * 注意：IRAM_ATTR表示这个函数存储在RAM中，可以更快地响应中断
 */
IRAM_ATTR void motionISR() {
    isMove = true;                      // 设置运动检测标志为真（检测到运动）
    lastMotionTime = millis();          // 记录当前时间（millis()返回系统启动后的毫秒数）
}

/**
 * KEY1中断服务函数 - 手动触发运动检测
 * 功能说明：当KEY1按钮被按下时，这个函数会被自动调用，模拟运动检测
 */
IRAM_ATTR void key1ISR() {
    isMove = true;                      // 设置运动检测标志为真（模拟检测到运动）
    lastMotionTime = millis();          // 记录当前时间
    Serial.println("KEY1按下 - 触发运动检测");  // 输出调试信息
}

/**
 * KEY2中断服务函数 - 手动消除运动检测
 * 功能说明：当KEY2按钮被按下时，这个函数会被自动调用，强制取消运动检测状态
 */
IRAM_ATTR void key2ISR() {
    isMove = false;                     // 设置运动检测标志为假（取消运动检测状态）
    Serial.println("KEY2按下 - 消除运动检测");  // 输出调试信息
}

/**
 * 根据环境光强度计算基础亮度
 * 功能说明：输入环境光照度值，输出对应的LED基础亮度值
 * 参数：lux - 环境光照度值（单位：勒克斯）
 * 返回值：LED亮度值（0-255）
 *
 * 亮度分档规则：
 * - 500lux以上：环境光充足，关闭灯光（返回0）
 * - 300-500lux：中等光线，设置为50亮度
 * - 100-300lux：较暗环境，设置为80亮度
 * - 100lux以下：很暗环境，设置为110亮度
 */
uint8_t calculateBaseBrightness(float Lux) {
    if (Lux >= LUX_THRESHOLD_HIGH) {    // 如果环境光照度 >= 500lux
        // 环境光充足，关闭灯光
        return 0;
    }
    else if (Lux >= LUX_THRESHOLD_MID) { // 如果环境光照度在300-500lux之间
        // 500lux档位：中等亮度
        return BRIGHTNESS_HIGH_LUX;     // 返回50
    }
    else if (Lux >= LUX_THRESHOLD_LOW) { // 如果环境光照度在100-300lux之间
        // 300lux档位：较高亮度
        return BRIGHTNESS_MID_LUX;      // 返回80
    }
    else {                              // 如果环境光照度 < 100lux
        // 100lux档位：最高基础亮度
        return BRIGHTNESS_LOW_LUX;      // 返回110
    }
}

/**
 * 更新亮度值，实现平滑变化曲线
 * 功能说明：这个函数每50毫秒被调用一次，负责：
 * 1. 检查运动检测是否超时
 * 2. 决定目标亮度
 * 3. 平滑地改变当前亮度，直到达到目标亮度
 *
 * 返回值：当前的LED亮度值（0-255）
 */
uint8_t updateBrightness() {
    uint32_t currentTime = millis();    // 获取当前系统时间（毫秒）

    /* ===== 步骤1：检查运动检测超时 ===== */
    // 如果检测到运动，但距离上次检测时间已经超过5秒，则取消运动检测状态
    if (isMove && (currentTime - lastMotionTime > motionTimeout)) {
        isMove = false;                 // 超时后自动取消运动检测状态
    }

    /* ===== 步骤2：确定目标亮度 ===== */
    if (baseBrightness > 0 && isMove) {     // 只有当环境亮度低于500lux（即baseBrightness > 0）且检测到运动时，才升至最高亮度
        targetBrightness = BRIGHTNESS_MAX;  // 设置目标亮度为最大值255
    }
    else {      // 其他情况下使用基础亮度（根据环境光计算的亮度）
        targetBrightness = baseBrightness;
    }

    /* ===== 步骤3：检查是否需要开始新的亮度变化过程 ===== */
    if (targetBrightness != currentBrightness) {        // 如果目标亮度与当前亮度不同
        if (targetBrightness > currentBrightness) {     // 需要增加亮度
            if (!isRising || isFalling) {               // 如果当前不在上升状态，或者正在下降，则开始新的上升过程
                isRising = true;                        // 设置为上升状态
                isFalling = false;                      // 取消下降状态
                brightnessStep = 0;                     // 重置步数计数器
                startBrightness = currentBrightness;    // 记录变化开始时的亮度值
            }
        }
        else {                                          // 需要降低亮度
            if (!isFalling || isRising) {               // 如果当前不在下降状态，或者正在上升，则开始新的下降过程
                isFalling = true;                       // 设置为下降状态
                isRising = false;                       // 取消上升状态
                brightnessStep = 0;                     // 重置步数计数器
                startBrightness = currentBrightness;    // 记录变化开始时的亮度值
            }
        }
    }

    /* ===== 步骤4：执行亮度变化（使用平滑曲线算法） ===== */
    if (isRising && currentBrightness < targetBrightness) {     // 如果正在上升且还没达到目标
        brightnessStep++;                                       // 增加步数计数器
        if (brightnessStep <= BRIGHTNESS_UP_STEPS) {            // 如果还在上升过程中（40步内）
            /* 使用平滑曲线算法：y = x^2，提供更自然的亮度变化，这种曲线的特点是：开始变化慢，后面变化快，符合人眼感觉 */
            float progress = (float) brightnessStep / (float) BRIGHTNESS_UP_STEPS;      // 计算当前进度（0.0到1.0）
            progress = progress * progress;                     // 应用平方曲线：progress^2，这样开始慢后面快
            /* 根据进度计算当前亮度值，公式：起始亮度 + (目标亮度 - 起始亮度) × 进度  */
            currentBrightness = startBrightness + (uint8_t) ((float) (targetBrightness - startBrightness) * progress);
        }
        else {                                      // 如果已经完成所有上升步骤
            currentBrightness = targetBrightness;   // 直接设置为目标亮度
            isRising = false;                       // 结束上升状态
        }
    }
    else if (isFalling && currentBrightness > targetBrightness) {   // 如果正在下降且还没达到目标
        brightnessStep++;                                           // 增加步数计数器
        if (brightnessStep <= BRIGHTNESS_DOWN_STEPS) {              // 如果还在下降过程中（60步内）
            /* 使用反向平滑曲线：y = 1 - (1-x)^2，提供更自然的亮度变化，这种曲线的特点是：开始变化快，后面变化慢，适合下降过程 */
            float progress = (float) brightnessStep / (float) BRIGHTNESS_DOWN_STEPS;    // 计算当前进度（0.0到1.0）
            progress = 1.0f - (1.0f - progress) * (1.0f - progress);        // 应用反向平方曲线：1 - (1-progress)^2，这样开始快后面慢
            /* 根据进度计算当前亮度值，公式：起始亮度 - (起始亮度 - 目标亮度) × 进度 */
            currentBrightness = startBrightness - (uint8_t) ((float) (startBrightness - targetBrightness) * progress);
        }
        else {                                      // 如果已经完成所有下降步骤
            currentBrightness = targetBrightness;   // 直接设置为目标亮度
            isFalling = false;                      // 结束下降状态
        }
    }
    else {                      // 已经到达目标亮度，或者不需要变化
        isRising = false;       // 确保清除上升标志
        isFalling = false;      // 确保清除下降标志
    }

    return currentBrightness;   // 返回当前的亮度值
}

/**
 * 根据环境光强度计算并更新当前亮度
 * 功能说明：这是主要的对外接口函数，整合了基础亮度计算和亮度平滑更新
 * 参数：lux - 环境光照度值
 * 返回值：当前应该设置的LED亮度值
 *
 * 调用流程：
 * 1. 根据环境光照度计算基础亮度
 * 2. 调用亮度更新函数，实现平滑变化
 * 3. 返回最终的亮度值给LED控制系统
 */
uint8_t calculatePerceivedBrightness(float Lux) {
    baseBrightness = calculateBaseBrightness(Lux);  // 第一步：根据环境光更新基础亮度
    return updateBrightness();                      // 第二步：更新并返回当前亮度值
}
