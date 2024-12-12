#pragma once
#include <main.h>

class Keyboard4x4 {
private:
    static constexpr uint16_t get_gpio_pin(uint8_t index) { return (0x0001 * (1 << index % 4)) << (index & ~0b11); }

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
};