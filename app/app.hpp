#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t huart1_receive_buffer[100];
extern uint8_t huart2_receive_buffer[100];
void entrypoint();
void initialize();
bool verify_password();

#ifdef __cplusplus
inline bool if_locked;
}
#endif