/** Generated Roboto Thin 15
 * @copyright Google https://github.com/googlefonts/roboto
 * @license This font is licensed under the Apache License, Version 2.0.
 */
#pragma once
#include "app/oled/oled_conf.hpp"
#include "main.h"

namespace oled::fonts {

typedef struct {
    const uint8_t width; /**< Font width in pixels */
    const uint8_t height; /**< Font height in pixels */
    const uint16_t* const data; /**< Pointer to font data array */
    const uint8_t* const char_width; /**< Proportional character width in pixels (NULL for monospaced) */
} Font;

#ifdef SSD1306_INCLUDE_FONT_6x8
extern const Font Font_6x8;
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
extern const Font Font_7x10;
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
extern const Font Font_11x18;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x26
extern const Font Font_16x26;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x24
extern const Font Font_16x24;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x15
extern const SSD1306_Font_t Font_16x15;
#endif

}