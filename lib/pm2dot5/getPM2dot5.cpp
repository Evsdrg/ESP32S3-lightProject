//
// PM2.5传感器数据接收模块
// 文件名: getPM2dot5.cpp
// 创建者: cepvor
// 创建时间: 2025/9/28
// 功能说明: 通过串口2接收PM2.5传感器数据，解析数据包并计算浓度值
// 硬件配置:
//   - TX引脚: IO17 (ESP32发送，实际上本项目中不需要发送)
//   - RX引脚: IO18 (ESP32接收PM2.5传感器数据)
//   - 串口: UART2
//   - 波特率: 9600
//   - 数据格式: 8位数据位，无校验位，1位停止位
// 数据协议:
//   - 帧格式: [0xA5][DATAH][DATAL][SUM]
//   - 0xA5: 帧头标识
//   - DATAH: PM2.5浓度高7位
//   - DATAL: PM2.5浓度低7位
//   - SUM: 校验字节(前3字节累加和的低7位)
//   - 浓度计算公式: PM2.5 = DATAH × 128 + DATAL
//

#include "getPM2dot5.h"

// ==================== 全局变量定义 ====================
uint16_t pm25_concentration = 0;     // PM2.5浓度值，单位：μg/m³，供其他模块使用
bool pm25_data_ready = false;        // 数据就绪标志，表示是否有新的有效数据

// ==================== 静态变量定义 ====================
static uint8_t rx_buffer[PM25_PACKET_SIZE];    // 串口接收缓冲区，存储一个完整数据包
static int buffer_index = 0;                   // 缓冲区索引，指示当前接收位置
static bool header_found = false;              // 帧头发现标志，用于状态机控制

/**
 * @brief 初始化PM2.5传感器串口通信
 * @details 配置串口2参数，设置TX/RX引脚，初始化相关变量
 *          串口配置：9600波特率，8位数据位，无校验位，1位停止位
 *          引脚配置：RX=IO18，TX=IO17
 */
void pm25_init() {
    /* 初始化串口2，配置波特率和引脚，SERIAL_8N1: 8位数据位，无校验位，1位停止位 */
    Serial2.begin(PM25_BAUD_RATE, SERIAL_8N1, PM25_RX_PIN, PM25_TX_PIN);
    /* 初始化状态变量，确保系统处于干净状态 */
    buffer_index = 0;                           // 重置缓冲区索引
    header_found = false;                       // 重置帧头搜索状态
    pm25_data_ready = false;                    // 重置数据就绪标志
    pm25_concentration = 0;                     // 重置浓度值
}

/**
 * @brief 校验接收到的数据包
 * @details 根据协议要求，计算前三个字节(header+dataH+dataL)的累加和的低7位
 *          与接收到的校验字节进行比较，验证数据包完整性
 * @param packet 指向数据包结构体的指针
 * @return true 校验通过，数据包有效
 * @return false 校验失败，数据包损坏
 */
static bool validate_packet(pm25_packet_t *packet) {
    /* 计算校验和：前三个字节累加和的低7位(& 0x7F 相当于 % 128) */
    uint8_t calculated_sum = (packet->header + packet->dataH + packet->dataL) & 0x7F;
    return (calculated_sum == packet->checksum);    // 将计算结果与接收到的校验字节比较
}

/**
 * @brief 根据高低字节计算PM2.5浓度值
 * @details 按照传感器协议，浓度值 = 高字节 × 128 + 低字节
 *          例：DATAH=0x01, DATAL=0x2C 时，浓度 = 1×128+44 = 172 μg/m³
 * @param dataH PM2.5浓度值高7位
 * @param dataL PM2.5浓度值低7位
 * @return uint16_t 计算得到的PM2.5浓度值，单位μg/m³
 */
static uint16_t calculate_concentration(uint8_t dataH, uint8_t dataL) {
    // 应用协议规定的计算公式
    return (uint16_t) (dataH * 128 + dataL);
}

/**
 * @brief 处理接收到的完整数据包
 * @details 对完整数据包进行校验，如果校验通过则计算浓度值并更新全局变量
 *          同时输出调试信息到串口监视器
 */
static void process_packet() {
    pm25_packet_t *packet = (pm25_packet_t *) rx_buffer;    // 将接收缓冲区强制转换为数据包结构体指针，便于访问各字段
    if (validate_packet(packet)) {  // 验证数据包完整性
        pm25_concentration = calculate_concentration(packet->dataH, packet->dataL); // 校验通过，计算并更新浓度值
        pm25_data_ready = true;     // 设置数据就绪标志
    }
}

/**
 * @brief 更新PM2.5数据（主循环调用）
 * @details 实现状态机逻辑处理串口数据接收：
 *          状态1：搜索帧头(0xA5)
 *          状态2：接收数据字节直到完整数据包
 *          使用状态机可以正确处理数据流中的任意位置开始接收
 * @note 此函数需要在主循环中频繁调用以确保及时处理串口数据
 */
void pm25_update() {
    while (Serial2.available()) {       // 处理串口缓冲区中的所有可用字节
        uint8_t byte = Serial2.read();  // 读取一个字节
        if (!header_found) {            // 状态1：搜索帧头阶段
            if (byte == PM25_HEADER) {  // 找到帧头0xA5
                rx_buffer[0] = byte;    // 保存帧头到缓冲区
                buffer_index = 1;       // 设置下一个字节的存储位置
                header_found = true;    // 切换到数据接收状态
            }
            /* 如果不是帧头，继续搜索（忽略当前字节） */
        }
        else {                                  // 状态2：数据接收阶段
            rx_buffer[buffer_index] = byte;     // 存储数据字节
            buffer_index++;                     // 移动到下一个位置
            if (buffer_index >= PM25_PACKET_SIZE) { // 检查是否接收到完整数据包(4字节)
                process_packet();                   // 处理完整数据包
                buffer_index = 0;                   // 重置状态机，准备接收下一个数据包
                header_found = false;
            }
        }
    }
}

/**
 * @brief 获取当前PM2.5浓度值
 * @details 返回最近一次成功解析的PM2.5浓度值
 *          此函数可被其他模块调用以获取PM2.5数据
 * @return uint16_t 当前PM2.5浓度值，单位μg/m³
 */
uint16_t pm25_get_value() {
    return pm25_concentration;
}

/**
 * @brief 检查是否有新的PM2.5数据
 * @details 检查数据就绪标志，用于判断是否有新的有效数据
 *          读取后会自动清除标志，避免重复处理同一数据
 * @return true 有新数据可用
 * @return false 没有新数据
 * @note 此函数采用"读取即清除"机制，每次调用后标志位会被清零
 */
bool pm25_is_data_ready() {
    bool ready = pm25_data_ready;
    pm25_data_ready = false;  // 读取后清除标志，避免重复处理
    return ready;
}
