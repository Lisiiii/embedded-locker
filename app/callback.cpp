#include "usart.h"

void HAL_USART_RxCpltCallback(UART_HandleTypeDef* husart) {
    if (husart == &huart2) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
}