#include "app/app.hpp"
#include "app/keyboard/keyboard4x4_driver.hpp"
#include "app/oled/font_library.hpp"
#include "app/oled/oled_driver.hpp"
#include <array>
#include <string>

void entrypoint() {
    Keyboard4x4 keyboard_reader;
    oled::SSD1306 oled { (uint8_t)255 };

    oled.clear_screen();
    oled.draw_string((char*)"Please enter your password:", oled::fonts::Font_7x10, 0);
    oled.draw_string((char*)"", oled::fonts::Font_7x10, 1);
    while (true) {
        Keyboard4x4::KeyCode current_key = keyboard_reader.read_keys();
        if (current_key != Keyboard4x4::KeyCode::NONE && keyboard_reader.key_changed) {
            HAL_UART_Transmit(&huart2, (uint8_t*)&current_key, sizeof(current_key), 100);
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }
    }
}
