#include "app/app.hpp"
#include "app/fingerprint_reader/fingerprint_reader_driver.hpp"
#include "app/keyboard/keyboard4x4_driver.hpp"
#include "app/oled/font_library.hpp"
#include "app/oled/oled_driver.hpp"
#include "fingerprint_reader/fingerprint_reader_driver.hpp"

#include <cstdint>
#include <main.h>
#include <map>
#include <math.h>
#include <string>
#include <usart.h>

uint8_t huart1_receive_buffer[100];
uint8_t huart2_receive_buffer[100];
bool if_locked = true;
bool if_ui_changed = true;

/* 1: fingerprint, 2: password, 3:app
 */
int unlock_way = 0;
uint32_t last_unlock_time = 0;

void initialize() {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, huart1_receive_buffer, sizeof(huart1_receive_buffer));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, huart2_receive_buffer, sizeof(huart2_receive_buffer));
    HAL_Delay(100); // wait for the devices to be ready
    oled::oled_1306.OLED_Init(0xEF);
    oled::oled_1306.fill();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    fingerprint_reader::zw101_FpReader.clear_fingerprint();
}

void entrypoint() {
    initialize();

    while (true) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, if_locked ? GPIO_PIN_SET : GPIO_PIN_RESET);
        keyboard::KeyCode current_key = keyboard::keyboard_reader.read_keys();
        unlock_way = if_locked ? 3 : unlock_way;

        if (HAL_GPIO_ReadPin(FINGER_TOUCH_GPIO_Port, FINGER_TOUCH_Pin) == GPIO_PIN_SET) {
            if_ui_changed = true;
            if_locked = !fingerprint_reader::zw101_FpReader.verify_fingerprint();
            unlock_way = if_locked ? 3 : 1;
        } else if (current_key == keyboard::KeyCode::KEY_D && keyboard::keyboard_reader.key_changed) {
            if_ui_changed = true;
            fingerprint_reader::zw101_FpReader.register_fingerprint(1, 1, 1);
        } else if (current_key == keyboard::KeyCode::KEY_E && keyboard::keyboard_reader.key_changed) {
            if_ui_changed = true;
            keyboard::keyboard_reader.change_password();
        } else if (current_key != keyboard::KeyCode::NONE && keyboard::keyboard_reader.key_changed) {
            if_ui_changed = true;
            if_locked = !keyboard::keyboard_reader.verify_password();
            unlock_way = if_locked ? 3 : 2;
        }

        if (if_ui_changed) {
            oled::oled_1306.clear_screen();
            oled::oled_1306.draw_string(if_locked ? "- Locked" : "- Unlocked", oled::fonts::Font_11x18, 0);
            oled::oled_1306.draw_string((char*)"", oled::fonts::Font_11x18, 1);
            oled::oled_1306.draw_string((char*)"---------------------", oled::fonts::Font_6x8, 0, 1);
            oled::oled_1306.draw_string((char*)"Touch the fingerprint sensor or enter password", oled::fonts::Font_6x8, 1);
            if_ui_changed = false;

            uint8_t upload_info[] = { static_cast<uint8_t>(if_locked ? 0x01 : 0x00), static_cast<uint8_t>(unlock_way) };
            HAL_UART_Transmit_DMA(&huart1, upload_info, sizeof(upload_info));
        }

        if (!if_locked && HAL_GetTick() - last_unlock_time > 30000) {
            if_locked = true;
            if_ui_changed = true;
            last_unlock_time = HAL_GetTick();
        }
    }
}
