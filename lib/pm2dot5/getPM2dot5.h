//
// Created by cepvor on 2025/9/28.
//

#ifndef LIGHTPROJECT_GETPM2DOT5_H
#define LIGHTPROJECT_GETPM2DOT5_H

#include <Arduino.h>

/* PM2.5数据包格式定义 */
#define PM25_HEADER 0xA5
#define PM25_PACKET_SIZE 4
#define PM25_TX_PIN 17
#define PM25_RX_PIN 18
#define PM25_BAUD_RATE 9600

/* PM2.5数据结构 */
typedef struct {
    uint8_t header;     // 0xA5
    uint8_t dataH;      // 浓度值高7位
    uint8_t dataL;      // 浓度值低7位
    uint8_t checksum;   // 校验字节
} pm25_packet_t;

/* 全局变量声明 */
extern uint16_t pm25_concentration;  // PM2.5浓度值，供其他代码使用
extern bool pm25_data_ready;         // 数据就绪标志

/* 函数声明 */
void pm25_init();
void pm25_update();
uint16_t pm25_get_value();
bool pm25_is_data_ready();

#endif //LIGHTPROJECT_GETPM2DOT5_H