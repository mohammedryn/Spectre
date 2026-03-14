#pragma once

#include <stdint.h>

void INA219_init();
int16_t INA219_read_current_raw();
bool INA219_is_ready();
