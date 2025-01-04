#pragma once

#include "app/oled/oled_driver.hpp"
#include <main.h>
#include <map>
#include <string>

namespace keyboard {
enum class KeyCode : uint16_t {
    KEY_1 = 0x0001,
    KEY_2 = 0x0002,
    KEY_3 = 0x0004,
    KEY_C = 0x0008,
    KEY_4 = 0x0010,
    KEY_5 = 0x0020,
    KEY_6 = 0x0040,
    KEY_D = 0x0080,
    KEY_7 = 0x0100,
    KEY_8 = 0x0200,
    KEY_9 = 0x0400,
    KEY_E = 0x0800,
    KEY_A = 0x1000,
    KEY_0 = 0x2000,
    KEY_B = 0x4000,
    KEY_F = 0x8000,
    NONE = 0
};

inline std::map<keyboard::KeyCode, int> key_name = {
    { keyboard::KeyCode::KEY_1, 1 },
    { keyboard::KeyCode::KEY_2, 2 },
    { keyboard::KeyCode::KEY_3, 3 },
    { keyboard::KeyCode::KEY_4, 4 },
    { keyboard::KeyCode::KEY_5, 5 },
    { keyboard::KeyCode::KEY_6, 6 },
    { keyboard::KeyCode::KEY_7, 7 },
    { keyboard::KeyCode::KEY_8, 8 },
    { keyboard::KeyCode::KEY_9, 9 },
    { keyboard::KeyCode::KEY_0, 0 },
    { keyboard::KeyCode::KEY_A, 10 },
    { keyboard::KeyCode::KEY_B, 11 },
    { keyboard::KeyCode::KEY_C, 12 },
    { keyboard::KeyCode::KEY_D, 13 },
    { keyboard::KeyCode::KEY_E, 14 },
    { keyboard::KeyCode::KEY_F, 15 },
};

class Keyboard4x4 {
private:
    static constexpr uint16_t get_gpio_pin(uint8_t index) { return (0x0001 * (1 << index % 4)) << (index & ~0b11); }
    bool if_last_verified = false;

    void set_row(int row) {
        for (int i = 0; i < 4; i++) {
            HAL_GPIO_WritePin(GPIOA, get_gpio_pin(i + 9), row == i ? GPIO_PIN_RESET : GPIO_PIN_SET);
        }
    }
    uint8_t read_col() {
        uint8_t col_code = 0;
        for (int i = 0; i < 4; i++) {
            if (HAL_GPIO_ReadPin(GPIOB, get_gpio_pin(i + 12)) == GPIO_PIN_RESET) {
                HAL_Delay(10);
                if (HAL_GPIO_ReadPin(GPIOB, get_gpio_pin(i + 12)) == GPIO_PIN_RESET) {
                    col_code |= 1 << i;
                }
            }
        };
        return col_code;
    };
    uint16_t last_keycode = 0;

public:
    Keyboard4x4() = default;

    int correct_password[4] = { 2, 2, 5, 6 };
    int current_password[4] = { 0, 0, 0, 0 };

    bool key_changed = false;
    KeyCode read_keys() {
        uint16_t key_code = 0;
        for (int i = 0; i < 4; i++) {
            set_row(i);
            uint8_t col_code = read_col();
            key_code |= col_code << (i * 4);
        }
        key_changed = (key_code != last_keycode);
        last_keycode = key_code;

        return static_cast<KeyCode>(key_code);
    };

    void change_password() {
        int index = 0;
        oled::oled_1306.clear_screen();
        oled::oled_1306.draw_string((char*)"Please enter your 4-digit password.", oled::fonts::Font_7x10, 0);
        oled::oled_1306.draw_string((char*)"---------------------", oled::fonts::Font_6x8, 1, 0);
        oled::oled_1306.draw_string((char*)"", oled::fonts::Font_7x10, 1);
        while (true) {
            keyboard::KeyCode current_key = read_keys();
            if (current_key == keyboard::KeyCode::KEY_D) {
                oled::oled_1306.clear_screen();
                return;
            }
            if (current_key != keyboard::KeyCode::NONE && key_changed) {
                if (current_key == keyboard::KeyCode::KEY_A) {
                    current_password[index] = 0;
                    index = index - 1 < 0 ? 0 : index - 1;
                    current_password[index] = 0;
                    oled::oled_1306.clear_char();
                } else if (current_key == keyboard::KeyCode::KEY_C) {
                    index = 0;
                    for (int i = 0; i < 4; i++) {
                        current_password[i] = 0;
                    }
                    oled::oled_1306.clear_line();
                } else if (current_key == keyboard::KeyCode::KEY_B) {
                    for (int i = 0; i < 4; i++) {
                        correct_password[i] = current_password[i];
                    }

                    if_last_verified = true;
                    return;
                } else if (index < 4) {
                    current_password[index] = key_name[current_key];
                    if (if_last_verified) {
                        oled::oled_1306.clear_line();
                        if_last_verified = false;
                        index = 0;
                    }
                    oled::oled_1306.draw_string((char*)std::to_string(current_password[index]).c_str(), oled::fonts::Font_11x18, 0, 0);
                    index++;
                }
            }
        }
    }

    bool verify_password() {
        int index = 0;
        oled::oled_1306.clear_screen();
        oled::oled_1306.draw_string((char*)"Please enter your 4-digit password.", oled::fonts::Font_7x10, 0);
        oled::oled_1306.draw_string((char*)"---------------------", oled::fonts::Font_6x8, 1, 0);
        oled::oled_1306.draw_string((char*)"", oled::fonts::Font_7x10, 1);
        while (true) {
            keyboard::KeyCode current_key = read_keys();
            if (current_key == keyboard::KeyCode::KEY_D) {
                oled::oled_1306.clear_screen();
                return false;
            }
            if (current_key != keyboard::KeyCode::NONE && key_changed) {
                if (current_key == keyboard::KeyCode::KEY_A) {
                    current_password[index] = 0;
                    index = index - 1 < 0 ? 0 : index - 1;
                    current_password[index] = 0;
                    oled::oled_1306.clear_char();
                } else if (current_key == keyboard::KeyCode::KEY_C) {
                    index = 0;
                    for (int i = 0; i < 4; i++) {
                        current_password[i] = 0;
                    }
                    oled::oled_1306.clear_line();
                } else if (current_key == keyboard::KeyCode::KEY_B) {
                    if (std::equal(current_password, current_password + 4, correct_password)) {
                        oled::oled_1306.draw_string((char*)"Passed!", oled::fonts::Font_11x18, 0, 1);
                        HAL_Delay(500);
                        for (int i = 0; i < 100; i += 3) {
                            oled::oled_1306.draw_circle(61, 61, i, 1);
                        }
                        for (int i = 0; i < 100; i += 3) {
                            oled::oled_1306.draw_circle(61, 0, i, 1, 1);
                        }
                        oled::oled_1306.clear_screen();
                        return true;
                    } else {
                        oled::oled_1306.draw_string((char*)"incorrect!", oled::fonts::Font_11x18, 0, 1);
                    }
                    if_last_verified = true;
                } else {
                    if (index < 4) {
                        current_password[index] = key_name[current_key];
                        if (if_last_verified) {
                            oled::oled_1306.clear_line();
                            if_last_verified = false;
                            index = 0;
                        }
                        oled::oled_1306.draw_string((char*)std::to_string(current_password[index]).c_str(), oled::fonts::Font_11x18, 0, 0);
                        index++;
                    } else {
                        if (if_last_verified) {
                            oled::oled_1306.clear_line();
                            if_last_verified = false;
                            index = 0;
                            for (int i = 0; i < 4; i++) {
                                current_password[i] = 0;
                            }
                            current_password[index] = key_name[current_key];
                            oled::oled_1306.draw_string((char*)std::to_string(current_password[index]).c_str(), oled::fonts::Font_11x18, 0, 0);
                            index++;
                        }
                    }
                }
            }
        }
    }
};
inline constinit Keyboard4x4 keyboard_reader;
}