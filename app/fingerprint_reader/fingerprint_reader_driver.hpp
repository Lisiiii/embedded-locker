#pragma once

#include "app/keyboard/keyboard4x4_driver.hpp"
#include "app/oled/oled_driver.hpp"
#include "usart.h"
#include <cstdio>
#include <main.h>

namespace fingerprint_reader {

struct __attribute__((packed)) Header {
    uint16_t package_header;
    uint32_t device_address;
    uint8_t signal; // 01 cmd , 02 data , 08 end-data
    uint16_t length;
};
struct __attribute__((packed)) RX_Package {
    Header header;
    uint8_t verify_code;
    uint8_t* data;
    uint16_t sum_check;
};

enum class package_type : uint8_t {
    CMD = 0x01,
    DATA = 0x02,
    END_DATA = 0x08,
};

class FingerprintReader {
public:
    FingerprintReader() = default;
    int fingerprint_count = 0;
    uint8_t ack = 0xFF;
    RX_Package ack_package = { { 0, 0, 0, 0 }, 0, nullptr, 0 };

    bool detect_finger() { return HAL_GPIO_ReadPin(FINGER_TOUCH_GPIO_Port, FINGER_TOUCH_Pin) == GPIO_PIN_SET; };

    void receive(uint8_t* buffer, size_t buffer_size) {
        RX_Package* package;
        package = reinterpret_cast<RX_Package*>(buffer);
        ack = package->verify_code;
        ack_package = *package;
    }

    void register_fingerprint(bool if_allow_override = false, bool if_allow_mult_register = true, bool if_allow_not_leave_device = true) {
        uint8_t config = if_allow_override << 3 | !if_allow_mult_register << 4 | if_allow_not_leave_device << 5 | 0x05;

        ack = 0xFF;
        oled::oled_1306.clear_screen();
        oled::oled_1306.draw_string((char*)"Press Key D to start fingerprint register", oled::fonts::Font_7x10, 0, 1);
        while (true) {
            if (keyboard::keyboard_reader.read_keys() == keyboard::KeyCode::KEY_D && keyboard::keyboard_reader.key_changed) break;
        }

        oled::oled_1306.clear_screen();
        oled::oled_1306.draw_string((char*)"Fingerprint register start", oled::fonts::Font_7x10, 0, 1);
        oled::oled_1306.draw_string((char*)"-----------------", oled::fonts::Font_7x10, 1, 0);
        oled::oled_1306.draw_string((char*)"", oled::fonts::Font_7x10, 1, 0);
        fingerprint_count += 1;
        uint8_t get_image[] = { 0x31, 0x00, (uint8_t)fingerprint_count, 0x02, 0x00, config };
        send_cmd(get_image);
        HAL_Delay(700);

        if (ack != 0x00) {
            error_code_process(ack);
            return;
        }
        oled::oled_1306.draw_string((char*)"start register...", oled::fonts::Font_7x10, 0, 1);

        HAL_Delay(2000);
        if (ack == 0x00) {
            oled::oled_1306.draw_string((char*)"Register successful!", oled::fonts::Font_7x10, 0, 1);
            HAL_Delay(1000);
        } else {
            error_code_process(ack);
        }
    }

    bool verify_fingerprint() {
        ack = 0xFF;
        oled::oled_1306.clear_screen();
        oled::oled_1306.draw_string((char*)"Fingerprint Verify", oled::fonts::Font_7x10, 0, 1);
        oled::oled_1306.draw_string((char*)"-----------------", oled::fonts::Font_7x10, 1, 0);
        oled::oled_1306.draw_string((char*)"", oled::fonts::Font_7x10, 1, 0);

        uint8_t get_image[] = { 0x32, 0x01, 0xff, 0xff, 0x00, 0x05 };
        send_cmd(get_image);
        HAL_Delay(700);

        if (ack != 0x00) {
            error_code_process(ack);
            return false;
        }
        ack = 0xFF;
        oled::oled_1306.draw_string((char*)"Matched!", oled::fonts::Font_11x18, 0, 1);
        HAL_Delay(500);
        return true;
    }

    void clear_fingerprint() {
        ack = 0xFF;

        uint8_t get_image[] = { 0x0d };
        send_cmd(get_image);
        HAL_Delay(100);

        if (ack != 0x00) {
            error_code_process(ack);
            return;
        }
        ack = 0xFF;
        oled::oled_1306.draw_string((char*)"Cleared!", oled::fonts::Font_11x18, 0, 1);
    }

private:
    void error_code_process(uint8_t error_code) {
        char error_message[50];
        switch (error_code) {
        case 0x02: {
            oled::oled_1306.draw_string((char*)"No finger!", oled::fonts::Font_7x10, 0, 1);
            break;
        }
        case 0x09: {
            oled::oled_1306.draw_string((char*)"Finger not match!", oled::fonts::Font_7x10, 0, 1);
            break;
        }
        case 0xFF: {
            oled::oled_1306.draw_string((char*)"Timeout...", oled::fonts::Font_7x10, 0, 1);
            break;
        }
        case 0x24: {
            oled::oled_1306.draw_string((char*)"Fingerprint library is empty", oled::fonts::Font_7x10, 0, 1);
            break;
        }
        default: {
            snprintf(error_message, sizeof(error_message), "Error code: %d", error_code);
            oled::oled_1306.draw_string(error_message, oled::fonts::Font_7x10, 0, 1);
        }
        }
        HAL_Delay(700);
    }

    uint16_t sum_check(const Header& header, uint8_t* data, size_t cmd_array_length) {
        uint16_t sum_check = 0;
        sum_check += header.length;
        sum_check += header.signal;
        for (unsigned int i = 0; i < cmd_array_length; i++)
            sum_check += data[i];
        return sum_check;
    }

    /* 命令数据包格式
     *  2 bytes | 0xEF01 | package header
     *  4 bytes | 0xFFFFFFFF | device address
     *  1 byte | 0x01 | signal
     *  2 bytes | N | length
     *  N bytes | cmd_array | cmd
     *  2 bytes | sum_check | sum check
     *  这里的端序是小端序，需要做一下转换
     */
    template <size_t N> void send_cmd(uint8_t (&cmd_array)[N]) {
        const size_t cmd_array_length = N;
        Header header { 0xEF01, 0xFFFFFFFF, static_cast<uint8_t>(package_type::CMD), static_cast<uint16_t>(cmd_array_length + 2) };

        uint16_t sum_check_result = swap_endian(sum_check(header, cmd_array, cmd_array_length));
        header.package_header = swap_endian(header.package_header);
        header.length = swap_endian(header.length);

        uint8_t package[sizeof(header) + cmd_array_length + 2] = { 0 };

        reinterpret_cast<Header&>(package[0]) = header;
        for (unsigned int i = 0; i < cmd_array_length; i++)
            package[sizeof(header) + i] = cmd_array[i];
        reinterpret_cast<uint16_t&>(package[sizeof(header) + cmd_array_length]) = sum_check_result;
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)&package, sizeof(package));
    }

    uint16_t swap_endian(uint16_t value) { return __builtin_bswap16(value); }
};
inline constinit FingerprintReader zw101_FpReader;
}