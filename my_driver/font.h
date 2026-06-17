#ifndef __FONT_H
#define __FONT_H

#include <linux/types.h>

typedef enum
{
    RGB565_WHITE    = (0xFFFF),                                                     // ��ɫ
    RGB565_BLACK    = (0x0000),                                                     // ��ɫ
    RGB565_BLUE     = (0x001F),                                                     // ��ɫ
    RGB565_PURPLE   = (0xF81F),                                                     // ��ɫ
    RGB565_PINK     = (0xFE19),                                                     // ��ɫ
    RGB565_RED      = (0xF800),                                                     // ��ɫ
    RGB565_MAGENTA  = (0xF81F),                                                     // Ʒ��
    RGB565_GREEN    = (0x07E0),                                                     // ��ɫ
    RGB565_CYAN     = (0x07FF),                                                     // ��ɫ
    RGB565_YELLOW   = (0xFFE0),                                                     // ��ɫ
    RGB565_BROWN    = (0xBC40),                                                     // ��ɫ
    RGB565_GRAY     = (0x8430),                                                     // ��ɫ

    RGB565_39C5BB   = (0x3616),
    RGB565_66CCFF   = (0x665F),
}rgb565_color_enum;

extern const uint8_t      ascii_font_8x16[][16];
extern const uint8_t      ascii_font_6x8[][6];
extern const uint8_t      chinese_test[8][16];
extern const uint8_t      oled_16x16_chinese[][16];
extern const uint8_t      gImage_seekfree_logo[38400];

extern const uint8_t gImage_game2[294];
extern const uint8_t sbdxq[][16];

#endif
