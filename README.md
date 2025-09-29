# ESP32-S3 智能亮度控制系统

## 项目概述
本项目基于ESP32-S3微控制器，采用PlatformIO + Arduino框架开发的智能亮度控制系统。系统集成了环境光传感器、运动检测传感器、温湿度传感器等多种传感器，通过MQTT协议实现远程监控和控制，支持自动亮度调节和人体感应功能。

## 核心功能特性

### 🌟 智能感应控制
- **环境光自适应**：根据环境光强度（500/300/100 lux三档）自动调节基础亮度
- **人体运动检测**：PIR传感器检测到运动时自动提升至最大亮度
- **平滑亮度变化**：采用非线性曲线算法，实现自然的亮度过渡效果
- **超时自动关闭**：运动检测5秒超时后自动恢复基础亮度

### 📡 远程通信功能
- **MQTT云端通信**：实时数据上报和远程控制命令接收
- **多传感器数据采集**：温度、湿度、光照强度实时监测
- **心跳包机制**：维持设备在线状态，确保连接稳定性
- **JSON数据格式**：标准化的数据交换格式

### 🔧 系统特性
- **多任务并行处理**：基于FreeRTOS的任务调度系统
- **看门狗保护**：防止系统死锁，自动故障恢复
- **硬件兼容性**：支持立创开发板和自制核心板
- **调试功能**：串口日志输出和手动控制按钮

## 硬件要求

### 必需组件
- ESP32-S3开发板（立创开发板或自制核心板）
- BH1750光照强度传感器（I2C接口）
- AHT20/AHT21温湿度传感器（I2C接口）
- PIR人体红外传感器
- WS2812B LED灯带或类似可编程LED

### 可选组件
- OLED显示屏（用于本地状态显示）
- 调试按钮（KEY1/KEY2，用于手动触发功能）

### 引脚连接说明
```
立创开发板引脚定义：
- 运动检测：GPIO16
- KEY1按钮：GPIO3（手动触发运动检测）
- KEY2按钮：GPIO4（手动消除运动检测）
- I2C_SDA：GPIO21
- I2C_SCL：GPIO22

自制核心板引脚定义：
- 运动检测：GPIO15  
- KEY1按钮：GPIO1
- KEY2按钮：GPIO2
- I2C_SDA：GPIO21
- I2C_SCL：GPIO22
```

## 项目结构
```
lightProject/
├── include/                    # 公共头文件目录
│   └── README
├── lib/                       # 功能模块库
│   ├── adcReading/           # ADC读取模块
│   ├── brightnessConfig/     # 亮度控制核心模块
│   ├── mqttConfig/           # MQTT通信模块
│   ├── oled/                 # OLED显示模块
│   ├── startInfo/            # 启动信息模块
│   ├── taskCreate/           # 任务创建管理模块
│   ├── timerManager/         # 定时器管理模块
│   └── wifiConfig/           # WiFi连接配置模块
├── src/
│   └── main.cpp              # 主程序入口
├── test/                     # 测试文件目录
├── platformio.ini            # PlatformIO项目配置
├── compile_commands.json     # 编译命令配置
└── README.md                 # 项目说明文档
```

## 快速开始

### 1. 环境准备
确保您的开发环境已安装：
- **PlatformIO IDE**（推荐VSCode + PlatformIO插件 或 CLion + PlatformIO插件）
- **ESP32工具链**（PlatformIO会自动安装）

### 2. 获取项目

- 将本项目放到合适的 **英文目录** 下
- 使用 VSCode 或者 CLion 导入项目

### 3. 配置项目参数

#### 🔧 WiFi连接配置
**文件位置**: `lib/wifiConfig/wifiConfig.h`

修改以下配置项：
```cpp
#define WIFI_SSID "您的WiFi名称"        // 替换为实际WiFi名称
#define WIFI_PASSWORD "您的WiFi密码"    // 替换为实际WiFi密码
```

#### 📡 MQTT服务器配置  
**文件位置**: `lib/mqttConfig/mqttConfig.h`

修改以下配置项：
```cpp
#define MQTT_BROKER_ADDR "192.168.1.114"    // MQTT服务器IP地址
#define MQTT_BROKER_PORT 1883               // MQTT服务器端口
#define MQTT_USERNAME "123"                 // MQTT用户名（可选）
#define MQTT_PASSWORD "456"                 // MQTT密码（可选）
#define DEVICE_ID "LIGHT_4"                 // 设备ID（确保唯一性）
```

#### 🎛️ 硬件平台配置
**文件位置**: `lib/taskCreate/taskCreate.h`

根据您的硬件配置修改：
```cpp
/* 开发板类型选择 */
#define isJLC false        // true=立创开发板, false=自制核心板

/* OLED显示屏启用 */  
#define useOLED true       // true=启用OLED显示, false=禁用OLED显示
```

### 4. 编译和上传
1. 在PlatformIO IDE中打开项目
2. 连接ESP32-S3开发板到电脑
3. 点击编译按钮检查代码
4. 点击上传按钮烧录固件

### 5. 运行和调试
- 打开串口监视器（波特率115200）查看运行日志
- 设备启动后会自动连接WiFi和MQTT服务器
- 观察环境光变化和运动检测的亮度响应

## 依赖库说明

项目使用的主要依赖库（PlatformIO会自动下载）：
- **Adafruit AHTX0** - 温湿度传感器驱动
- **BH1750** - 光照强度传感器驱动  
- **PubSubClient** - MQTT客户端库
- **FastLED** - 可编程LED控制库
- **ArduinoJson** - JSON数据处理库

## MQTT通信协议

### 主题定义
- **数据上报**: `device/{DEVICE_ID}/data`
- **心跳包**: `device/{DEVICE_ID}/heartbeat`  
- **控制命令**: `device/{DEVICE_ID}/control`

### 数据上报格式
```json
{
  "ambient_light": 245.6,
  "light_brightness": 75,
  "temperature": 26.8,
  "humidity": 65.2,
  "pm25": 0,
  "battery_level": 0,
  "solar_voltage": 0,
  "auto_mode": true
}
```

### 控制命令格式
```json
{
  "command": "set_brightness",
  "value": 128,
  "auto_mode": false
}
```

## 系统架构说明

### 任务调度策略
系统采用FreeRTOS多任务架构，主要任务包括：
- **核心0任务**：传感器数据采集、MQTT通信
- **核心1任务**：亮度控制、LED显示、OLED更新

### 亮度控制算法
采用三档环境光自适应算法：
- **≥500lux**: 关闭LED（环境光充足）
- **300-500lux**: 基础亮度50/255
- **100-300lux**: 基础亮度80/255  
- **<100lux**: 基础亮度110/255

运动检测时升至最大亮度255/255，5秒后自动恢复。

### 平滑变化曲线
- **上升过程**：2秒内完成，使用x²曲线（慢启动，快结束）
- **下降过程**：3秒内完成，使用1-(1-x)²曲线（快启动，慢结束）

## 故障排除

### 常见问题
1. **WiFi连接失败**
   - 检查WiFi名称和密码是否正确
   - 确认WiFi信号强度足够
   - 查看串口日志的详细错误信息

2. **MQTT连接失败**  
   - 确认MQTT服务器地址和端口正确
   - 检查网络连通性
   - 验证用户名密码（如果需要）

3. **传感器读取异常**
   - 检查I2C接线是否正确
   - 确认传感器电源供电正常
   - 查看串口是否有I2C错误信息

4. **LED不亮或异常**
   - 检查LED接线和电源
   - 确认LED类型配置正确
   - 验证GPIO引脚定义

### 调试技巧
- 启用串口日志查看详细运行信息
- 使用KEY1/KEY2按钮手动测试运动检测功能
- 通过MQTT客户端工具测试远程控制
- 检查看门狗重启日志判断系统稳定性

## 开发扩展

### 添加新传感器
1. 在`lib/`目录下创建新的传感器模块
2. 在`taskCreate.h`中声明相关任务
3. 修改MQTT数据上报格式

### 自定义亮度算法
修改`lib/brightnessConfig/brightnessConfig.cpp`中的算法逻辑

### 增加控制命令  
在`lib/mqttConfig/mqttConfig.cpp`中扩展命令处理函数

## 参考资源
- [ESP32-S3官方文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/)
- [PlatformIO使用指南](https://docs.platformio.org/)
- [MQTT协议规范](http://mqtt.org/)
- [FreeRTOS任务管理](https://www.freertos.org/taskandcr.html)

## 版本信息
- **项目版本**: 2.2
- **更新日期**: 2025/09/28
- **Arduino框架**: 适用于ESP32-S3
- **许可证**: MIT License

---
**注意**: 首次使用前请务必根据实际硬件情况修改上述三个关键配置文件，否则系统无法正常工作。
