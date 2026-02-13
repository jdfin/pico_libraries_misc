
#include <cstdint>
//
#include "sys_led.h"

bool SysLed::_ready = false;

uint32_t SysLed::_on_ms = 0;
uint32_t SysLed::_off_ms = 0;

uint64_t SysLed::_on_next_ms = UINT64_MAX;
uint64_t SysLed::_off_next_ms = UINT64_MAX;
