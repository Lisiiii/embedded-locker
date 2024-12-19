#pragma once
#include "usart.h"
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

    bool detect_finger() { return HAL_GPIO_ReadPin(FINGER_TOUCH_GPIO_Port, FINGER_TOUCH_Pin) == GPIO_PIN_SET; };

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

        HAL_UART_Transmit(&huart2, (uint8_t*)&package, sizeof(package), 100);
    }

    void receive() {
        RX_Package package;
        HAL_UART_Receive(&huart2, (uint8_t*)&package, sizeof(package), 100);
    }

private:
    uint16_t sum_check(const Header& header, uint8_t* data, size_t cmd_array_length) {
        uint16_t sum_check = 0;
        sum_check += header.length;
        sum_check += header.signal;
        for (unsigned int i = 0; i < cmd_array_length; i++)
            sum_check += data[i];
        return sum_check;
    }

    uint16_t swap_endian(uint16_t value) { return __builtin_bswap16(value); }
};
inline constinit FingerprintReader fingerprint_reader;
}