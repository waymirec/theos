#pragma once

#include <stdint.h>

void pit_init(uint32_t frequency);
void pit_tick(void);
void pit_sleep(uint64_t millis);
uint64_t pit_uptime(void);