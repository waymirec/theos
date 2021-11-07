#pragma once

#include <stdbool.h>

#define PS2_RES_ACK 0xFA
#define PS2_RES_TEST_PASSED 0xAA
#define PS2_RES_TEST_FAILED 0xFC
#define PS2_RES_DEVICE_ID 0x00

#define PS2_DATA_PORT 0x60
#define PS2_COMMAND_PORT 0x64
#define PS2_STATUS_REG 0x64

#define PS2_CMD_ENABLE 0xAE
#define PS2_CMD_DISABLE 0xAD
#define PS2_CMD_TEST 0xAB

#define PS2_CMD_ENABLE_AUX 0xA8
#define PS2_CMD_DISABLE_AUX 0xA7
#define PS2_CMD_TEST_AUX 0xA9

#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_DISABLE_SCANNING 0xF5
#define PS2_CMD_IDENT 0xF2

#define PS2_CMD_READ_CFG 0x20
#define PS2_CMD_WRITE_CFG 0x60
#define PS2_CMD_WRITE_AUX 0xD4

#define PS2_CMD_RESET 0xFF
#define PS2_CMD_RESEND 0xFE
#define PS2_CMD_SET_DEFAULTS 0xF6
#define PS2_CMD_SET_SAMPLE_RATE 0xF3
#define PS2_CMD_ENABLE_DATA_REPORTING 0xF4
#define PS2_CMD_DISABLE_DATA_REPORTING 0xF5

typedef struct {
    bool        y_overflow      : 1;              // y-axis overflow
    bool        x_overflow      : 1;              // x-axis overflow
    bool        y_negative      : 1;              // y value negative
    bool        x_negative      : 1;              // x value negative
    bool        reserved_0      : 1;              // always 1
    bool        middle_button   : 1;              // middle button pressed
    bool        right_button    : 1;              // right button pressed
    bool        left_button     : 1;              // left button pressed
    uint8_t     x_data;                           // motion units since last padcket
    uint8_t     y_data;                           // motion units since last packet
} __attribute__((packed)) mouse_data_s;