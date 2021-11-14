#include "pit.h"

#include "io.h"

#define PIT_CH0_DATA_PORT 0x40
#define PIT_CH1_DATA_PORT 0x41
#define PIT_CH2_DATA_PORT 0x42
#define PIT_CMDREG 0x43

const uint64_t BASE_FREQ = 1193182;
const uint64_t MILLIS_PER_SECOND = 1000;

uint64_t _ticks = 0;
uint16_t _divisor;
uint32_t _frequency;
uint64_t _time_since_boot = 0;

static void __set_divisor(uint16_t);

void pit_init(uint32_t frequency)
{
    _frequency = frequency;
    _divisor = BASE_FREQ / frequency;
    __set_divisor(_divisor);
}

void pit_tick(void)
{
    _ticks++;
    _time_since_boot += (MILLIS_PER_SECOND / (BASE_FREQ / _divisor));
    
}

void pit_sleep(uint64_t millis)
{
    uint64_t num_ticks = millis / (MILLIS_PER_SECOND / (BASE_FREQ / _divisor));
    uint64_t t = _ticks + num_ticks;
    while(_ticks < t) {
        asm("hlt");
    }
}

uint64_t pit_uptime(void)
{
    return _time_since_boot;
}

static void __set_divisor(uint16_t divisor)
{
    _divisor = divisor >= 100 ? divisor : 100;
    outb(PIT_CH0_DATA_PORT, (uint8_t)(divisor & 0x00FF));
    io_wait();
    outb(PIT_CH0_DATA_PORT, (uint8_t)((divisor & 0xFF00) >> 8));
}

