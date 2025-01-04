#pragma once
#include "app/oled/font_library.hpp"
#include <main.h>
#include <utility>

namespace oled {

typedef struct {
    int x;
    int y;
    int last_width;
    int last_height;
} FontCursor;

typedef struct {
    int x;
    int y;
} Cursor;

class SSD1306 {
public:
    // SSD1306(int contrast = 0xEF) { OLED_Init(contrast); };
    SSD1306() = default;
    int OLED_WIDTH;
    int OLED_HEIGHT;
    Cursor current_cursor;
    FontCursor font_cursor;

    void set_contrast(int contrast) {
        OLED_write(cmd_, 0x81);
        OLED_write(cmd_, (uint8_t)contrast);
    };

    /* 以屏幕像素点为单位，以左上角为原点 x:0~127 y:0~63 */
    void draw_frame(bool display_array[128][64]) {
        clear_screen();
        for (uint8_t i = 0; i < OLED_WIDTH; i++)
            for (uint8_t j = 0; j < OLED_HEIGHT; j++)
                set_pixel(i, j, (uint8_t)display_array[i][j]);
        GRAM_refresh();
    };

    /* 以屏幕像素点为单位，以左上角为原点 x:0~127 y:0~63 */
    void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool if_fill) {
        if (x1 > OLED_WIDTH || y1 > OLED_HEIGHT || x2 > OLED_WIDTH || y2 > OLED_HEIGHT) return;

        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        for (uint8_t i = x1; i <= x2; i++) {
            set_pixel(i, y1, 1);
            set_pixel(i, y2, 1);
        }
        for (uint8_t i = y1; i <= y2; i++) {
            set_pixel(x1, i, 1);
            set_pixel(x2, i, 1);
        }

        if (if_fill)
            for (uint8_t i = x1 + 1; i < x2; i++)
                for (uint8_t j = y1 + 1; j < y2; j++)
                    set_pixel(i, j, 1);

        GRAM_refresh();
    }

    /* 以屏幕像素点为单位，以左上角为原点 x:0~127 y:0~63 */
    void draw_circle(uint8_t centerX, uint8_t centerY, uint8_t radius, bool if_fill, bool if_inverse = false) {
        bool mode = !if_inverse;
        if (centerX >= OLED_WIDTH || centerY >= OLED_HEIGHT) {
            return;
        }

        int32_t x = -radius;
        int32_t y = 0;
        int32_t err = 2 - 2 * radius;
        int32_t e2;

        while (x <= 0) {
            int32_t px = centerX - x;
            int32_t nx = centerX + x;
            int32_t py = centerY + y;
            int32_t ny = centerY - y;

            if (px >= 0 && px < OLED_WIDTH) {
                if (py >= 0 && py < OLED_HEIGHT) set_pixel(px, py, mode);
                if (ny >= 0 && ny < OLED_HEIGHT) set_pixel(px, ny, mode);
            }

            if (nx >= 0 && nx < OLED_WIDTH) {
                if (py >= 0 && py < OLED_HEIGHT) set_pixel(nx, py, mode);
                if (ny >= 0 && ny < OLED_HEIGHT) set_pixel(nx, ny, mode);
            }

            e2 = err;
            if (e2 <= y) {
                y++;
                err += y * 2 + 1;
            }

            if (e2 > x) {
                x++;
                err += x * 2 + 1;
            }
        }

        if (if_fill)
            for (uint8_t i = 0; i < radius; i++)
                for (uint8_t j = 0; j < radius; j++)
                    if (i * i + j * j <= radius * radius) {
                        int32_t px = centerX - i;
                        int32_t nx = centerX + i;
                        int32_t py = centerY + j;
                        int32_t ny = centerY - j;

                        if (px >= 0 && px < OLED_WIDTH) {
                            if (py >= 0 && py < OLED_HEIGHT) set_pixel(px, py, mode);
                            if (ny >= 0 && ny < OLED_HEIGHT) set_pixel(px, ny, mode);
                        }

                        if (nx >= 0 && nx < OLED_WIDTH) {
                            if (py >= 0 && py < OLED_HEIGHT) set_pixel(nx, py, mode);
                            if (ny >= 0 && ny < OLED_HEIGHT) set_pixel(nx, ny, mode);
                        }
                    }

        GRAM_refresh();
    };
    void draw_string(const char* str, oled::fonts::Font font, bool if_enter, bool if_clearline = false) {
        if (if_clearline) {
            clear_line();
        } else if (if_enter) {
            font_cursor.x = 0;
            font_cursor.y += font.height;
        }

        while (*str) {
            draw_char(*str, font);
            str++;
        }
    }
    void fill() { draw_rectangle(0, 0, OLED_WIDTH, OLED_HEIGHT, 1); }
    void clear_screen() {
        for (uint8_t i = 0; i < 8; i++)
            for (uint8_t j = 0; j < OLED_WIDTH; j++)
                OLED_GRAM[j][i] = 0x00;
        font_cursor = { 0, 0, 0, 0 };
        GRAM_refresh();
    };

    void clear_line() {
        for (uint8_t i = 0; i < OLED_WIDTH; i++)
            for (uint8_t j = font_cursor.y; j < font_cursor.y + font_cursor.last_height; j++)
                set_pixel(i, j, 0);
        font_cursor.x = 0;
        GRAM_refresh();
    }
    void clear_char() {
        if (font_cursor.x - font_cursor.last_width < 0) {
            font_cursor.x = OLED_WIDTH;
            font_cursor.y = font_cursor.y - font_cursor.last_height > 0 ? font_cursor.y - font_cursor.last_height : 0;
        }
        for (uint8_t i = font_cursor.x; i > font_cursor.x - font_cursor.last_width; i--)
            for (uint8_t j = font_cursor.y; j < font_cursor.y + font_cursor.last_height; j++)
                set_pixel(i, j, 0);
        font_cursor.x = font_cursor.x - font_cursor.last_width;
        GRAM_refresh();
    }
    void OLED_Init(int contrast) {
        OLED_WIDTH = 128;
        OLED_HEIGHT = 64;
        current_cursor = { 64, 32 };
        font_cursor = { 0, 0, 0, 0 };
        cmd_ = 1;
        data_ = 0;

        OLED_CS(1);
        OLED_DC(1);

        /* 复位 */
        OLED_RES(0);
        HAL_Delay(1000);
        OLED_RES(1);

        /* 开始写入初始化命令 */
        OLED_write(cmd_, 0xAE); // 关闭显示
        OLED_write(cmd_, 0xD5); // 设置时钟分频因子
        OLED_write(cmd_, 80);

        OLED_write(cmd_, 0xA8); // 设置驱动路数
        OLED_write(cmd_, 0x3F); // 路数默认0x3F（1/64）

        OLED_write(cmd_, 0xD3); // 设置显示偏移
        OLED_write(cmd_, 0x00); // 偏移默认为0

        OLED_write(cmd_, 0x40); // 设置显示开始行[5:0]

        OLED_write(cmd_, 0x8D); // 电荷泵设置
        OLED_write(cmd_, 0x14); // bit2，开启/关闭

        OLED_write(cmd_, 0x20); // 设置内存地址模式
        OLED_write(cmd_, 0x02); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;

        OLED_write(cmd_, 0xA1); // 段重定义设置,bit0:0,0->0;1,0->127;
        OLED_write(cmd_, 0xC0); // 设置COM扫描方向;bit3:0,普通模式;1,重定义模式
                                // COM[N-1]->COM0;N:驱动路数
        OLED_write(cmd_, 0xDA); // 设置COM硬件引脚配置
        OLED_write(cmd_, 0x12); //[5:4]配置

        OLED_write(cmd_, 0x81); // 对比度设置
        OLED_write(cmd_, (uint8_t)contrast); // 默认0x7F（范围1~255，越大越亮）

        OLED_write(cmd_, 0xD9); // 设置预充电周期
        OLED_write(cmd_, 0xF1); //[3:0],PHASE 1;[7:4],PHASE 2;

        OLED_write(cmd_, 0xDB); // 设置VCOMH 电压倍率
        OLED_write(cmd_, 0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

        OLED_write(cmd_, 0xA4); // 全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
        OLED_write(cmd_, 0xA6); // 设置显示方式;bit0:1,反相显示;0,正常显示

        OLED_write(cmd_, 0xAF); // 开启显示

        /* 清屏函数 */
        clear_screen();
    }

private:
    uint8_t OLED_GRAM[128][8];

    bool cmd_;
    bool data_;

    void set_pixel(uint8_t x, uint8_t y, uint8_t mode) {
        if (x > 127 || y > 63) return;

        uint8_t i = 7 - y / 8; // 算出第几页
        uint8_t j = y % 8;
        uint8_t temp = 0x01 << (7 - j); // 由位运算精确找出坐标像素点

        mode ? OLED_GRAM[x][i] |= temp : OLED_GRAM[x][i] &= ~temp;
    };
    void draw_char(char ch, oled::fonts::Font font) {
        uint32_t i, b, j;
        // 检查字符是否有效
        if (ch < 32 || ch > 126) return;

        // 检查当前行剩余空间
        if (OLED_WIDTH < (font_cursor.x + font.width)) {
            font_cursor.x = 0;
            font_cursor.y += font.height;
        }
        if (OLED_HEIGHT < (font_cursor.y + font.height))
            // 当前行空间不足
            return;

        // 使用字体进行写入
        for (i = 0; i < font.height; i++) {
            b = font.data[(ch - 32) * font.height + i];
            for (j = 0; j < font.width; j++) {
                if ((b << j) & 0x8000) {
                    set_pixel(font_cursor.x + j, font_cursor.y + i, 1);
                } else {
                    set_pixel(font_cursor.x + j, font_cursor.y + i, 0);
                }
            }
        }
        // 声明当前空间已被占用
        font_cursor.x += font.char_width ? font.char_width[ch - 32] : font.width;
        font_cursor.last_width = font.width;
        font_cursor.last_height = font.height;
        GRAM_refresh();
    }

    void OLED_write(bool cmd, uint8_t data) {
        uint8_t i, k;
        // 拉低片选CS，写命令拉低DC
        OLED_CS(0);
        cmd ? OLED_DC(0) : OLED_DC(1);

        for (i = 0; i < 8; i++) {
            // 时钟线，上升沿有效
            OLED_SCLK(0);
            k = data & (0x80);
            OLED_SDIN(k);
            OLED_SCLK(1);
            data <<= 1;
        }
        OLED_CS(1);
        OLED_DC(1);
    }
    void GRAM_refresh() {
        for (int i = 0; i < 8; i++) {
            OLED_write(cmd_, 0xB0 + i); // 设置页地址（行）
            OLED_write(cmd_, 0x00); // 设置列地址的低四位
            OLED_write(cmd_, 0x10); // 设置列地址的高四位
            // 一些OLED前两行不显示任何东西，非常令人迷惑 ,前两行我们随便写点东西
            for (int j = -2; j < OLED_WIDTH; j++) {
                if (j < 0) OLED_write(data_, 0x00); // 前两行不起作用;
                else
                    OLED_write(data_, OLED_GRAM[j][i]); // 将GRAM中图像信息写入屏幕
            }
        }
    }
    void OLED_RES(bool cmd) { cmd ? HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET); }
    void OLED_DC(bool cmd) { cmd ? HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET); }
    void OLED_CS(bool cmd) { cmd ? HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); }
    void OLED_SCLK(bool cmd) { cmd ? HAL_GPIO_WritePin(OLED_SCK_GPIO_Port, OLED_SCK_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_SCK_GPIO_Port, OLED_SCK_Pin, GPIO_PIN_RESET); };
    void OLED_SDIN(bool cmd) { cmd ? HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, GPIO_PIN_RESET); };
};
inline constinit SSD1306 oled_1306;
}