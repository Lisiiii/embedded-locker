#include "app/app.hpp"

#include "app/keyboard/keyboard4x4_driver.hpp"
#include "app/oled/font_library.hpp"
#include "app/oled/oled_driver.hpp"
#include "fingerprint_reader/fingerprint_reader_driver.hpp"

#include <main.h>
#include <map>
#include <math.h>
#include <string>
#include <usart.h>

std::map<keyboard::KeyCode, int> key_name = {
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
int index = 0;
int correct_password[4] = { 1, 2, 3, 4 };
int current_password[4] = { 0, 0, 0, 0 };
bool if_last_verified = false;

uint8_t receive_buffer[100] = { 0 };

void entrypoint() {
    oled::oled_1306.OLED_Init(0xEF);
    while (true) {
        uint8_t cmd_array[] = { 0x34, 0x00 };
        fingerprint_reader::fingerprint_reader.send_cmd(cmd_array);
        HAL_Delay(1000);
        HAL_UART_Receive_IT(&huart2, receive_buffer, sizeof(receive_buffer));
        oled::oled_1306.fill();
        // if (fp_reader.detect_finger()) {
        //     HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        // } else {
        //     HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        // }
    }
}

// void password_verify() {

//     oled::oled.clear_screen();
//     oled::oled.draw_string((char*)"Please enter your 4-digit password:", oled::fonts::Font_7x10, 0);
//     oled::oled.draw_string((char*)"", oled::fonts::Font_7x10, 1);
//     while (true) {
//         keyboard::KeyCode current_key = keyboard::keyboard_reader.read_keys();
//         if (current_key != keyboard::KeyCode::NONE && keyboard::keyboard_reader.key_changed) {
//             HAL_UART_Transmit(&huart2, (uint8_t*)&current_key, sizeof(current_key), 100);
//             HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//             if (current_key == keyboard::KeyCode::KEY_A) {
//                 current_password[index] = 0;
//                 index = index - 1 < 0 ? 0 : index - 1;
//                 current_password[index] = 0;
//                 oled::oled.clear_char();
//             } else if (current_key == keyboard::KeyCode::KEY_C) {
//                 index = 0;
//                 for (int i = 0; i < 4; i++) {
//                     current_password[i] = 0;
//                 }
//                 oled::oled.clear_line();
//             } else if (current_key == keyboard::KeyCode::KEY_B) {
//                 oled::oled.draw_string((char*)"Loading...", oled::fonts::Font_11x18, 0, 1);
//                 HAL_Delay(1000);
//                 if (std::equal(current_password, current_password + 4, correct_password)) {
//                     oled::oled.draw_string((char*)"Passed!", oled::fonts::Font_11x18, 0, 1);
//                     HAL_Delay(500);
//                     for (int i = 0; i < 100; i += 3) {
//                         oled::oled.draw_circle(61, 61, i, 1);
//                     }
//                     for (int i = 0; i < 100; i += 3) {
//                         oled::oled.draw_circle(61, 0, i, 1, 1);
//                     }
//                     oled::oled.clear_screen();
//                     oled::oled.draw_string((char*)"UNLOCKED", oled::fonts::Font_16x24, 0, 1);
//                 } else {
//                     oled::oled.draw_string((char*)"incorrect!", oled::fonts::Font_11x18, 0, 1);
//                 }
//                 if_last_verified = true;
//             } else {
//                 if (index < 4) {
//                     current_password[index] = key_name[current_key];
//                     if (if_last_verified) {
//                         oled::oled.clear_line();
//                         if_last_verified = false;
//                         index = 0;
//                     }
//                     oled::oled.draw_string((char*)std::to_string(current_password[index]).c_str(), oled::fonts::Font_11x18, 0, 0);
//                     index++;
//                 } else {
//                     if (if_last_verified) {
//                         oled::oled.clear_line();
//                         if_last_verified = false;
//                         index = 0;
//                         for (int i = 0; i < 4; i++) {
//                             current_password[i] = 0;
//                         }
//                         current_password[index] = key_name[current_key];
//                         oled::oled.draw_string((char*)std::to_string(current_password[index]).c_str(), oled::fonts::Font_11x18, 0, 0);
//                         index++;
//                     }
//                 }
//             }
//         } else {
//             HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
//         }
//     }
// }
