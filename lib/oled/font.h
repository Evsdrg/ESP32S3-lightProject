/**
 * @file font.h
 * @brief 字模库头文件
 * @author cepvor
 * @version 1.35
 * @date 2025/9/16
 * @license MIT License
 *
 * @attention
 * 本文件为字模库头文件，包含如下内容：
 * - ASCII字体结构体及常用字体声明
 * - 字体结构体及常用字体声明
 * - 图片结构体及常用图片声明
 *
 * @note
 * 注意事项：
 * - 字库数据可以使用波特律动LED取模助手生成(https://led.baud-dance.com)
 * - 记得extern引用添加的字体和图片
 */

#ifndef FONT_H
#define FONT_H
#include <cstdint>
#include <cstring>
typedef struct ASCIIFont {
  uint8_t h;
  uint8_t w;
  uint8_t *chars;
} ASCIIFont;

extern const ASCIIFont afont8x6;
extern const ASCIIFont afont12x6;
extern const ASCIIFont afont16x8;
extern const ASCIIFont afont24x12;

/**
 * @brief 字体结构体
 * @note  字库前4字节存储utf8编码 剩余字节存储字模数据
 * @note 字库数据可以使用波特律动LED取模助手生成(https://led.baud-dance.com)
 */
typedef struct Font {
  uint8_t h;              // 字高度
  uint8_t w;              // 字宽度
  const uint8_t *chars;   // 字库 字库前4字节存储utf8编码 剩余字节存储字模数据
  uint8_t len;            // 字库长度 超过256则请改为uint16_t
  const ASCIIFont *ascii; // 缺省ASCII字体 当字库中没有对应字符且需要显示ASCII字符时使用
} Font;

extern const Font font16x16;
extern const Font font12x12;

/**
 * @brief 图片结构体
 * @note  图片数据可以使用波特律动LED取模助手生成(https://led.baud-dance.com)
 */
typedef struct Image {
  uint8_t w;           // 图片宽度
  uint8_t h;           // 图片高度
  const uint8_t *data; // 图片数据
} Image;

extern const Image bilibiliImg;
extern const Image bh313Img;
extern const Image temperatureImg;
extern const Image humidityImg;
extern const Image batteryImg;
extern const Image solarImg;
extern const Image PM2dot5Img;

#endif // FONT_H