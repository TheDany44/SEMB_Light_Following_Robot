#pragma once
#include <stdint.h>

typedef enum {
    CMD_NONE = -1,
    CMD_CONNECT = 0,
    CMD_START_STOP_TOGGLE = 1,
    CMD_CALIBRATE = 2,
    CMD_MANUAL_TOGGLE = 3,
    CMD_JOYSTICK_DATA = 4
} CommandType_t;

typedef struct {
    CommandType_t cmd;
    int16_t joyX;   // -512 .. +512
    int16_t joyY;   // -512 .. +512
} CommandPacket_t;