#include "app/app.hpp"
#include "app/fingerprint_reader/fingerprint_reader_driver.hpp"
#include "usart.h"
#include <cstring>

void process_esp32(uint16_t size) {
    HAL_UART_Transmit_DMA(&huart1, huart1_receive_buffer, size);
    if (strncmp((char*)huart1_receive_buffer, "lock", 4) == 0) {
        if_locked = true;
    } else if (strncmp((char*)huart1_receive_buffer, "unlock", 6) == 0) {
        if_locked = false;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    if (huart->RxEventType != HAL_UART_RXEVENT_IDLE) return;
    if (huart == &huart1) {
        process_esp32(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, huart1_receive_buffer, sizeof(huart1_receive_buffer));
    }
    if (huart == &huart2) {
        fingerprint_reader::zw101_FpReader.receive(huart2_receive_buffer, Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, huart2_receive_buffer, sizeof(huart2_receive_buffer));
    }
}
