#pragma once
#include <stdint.h>

#define NUM_LDRS 8

typedef enum {
    MODE_INIT,
    MODE_STOPPED,
    MODE_CALIBRATION,
    MODE_AUTO,
    MODE_MANUAL
} RobotMode_t;

typedef struct {
    RobotMode_t currentMode;

    uint16_t ultrasonicDistance;   // cm
    uint8_t obstacleDetected;      // 0 or 1

    uint16_t ldrRaw[NUM_LDRS];
    uint16_t ldrOffset[NUM_LDRS];

    int8_t lightDirection;

    uint32_t t_ultrasonic;
    uint32_t t_max_ultrasonic;
    uint32_t t_motor;
    uint32_t t_max_motor;
    uint32_t t_auto;
    uint32_t t_max_auto;
    uint32_t t_manual;
    uint32_t t_max_manual;
    uint32_t t_light;
    uint32_t t_max_light;
    uint32_t t_ldr;
    uint32_t t_max_ldr;
    uint32_t t_calibration;
    uint32_t t_max_calibration;
    uint32_t t_systemmode;
    uint32_t t_max_systemmode;
    uint32_t t_espnowrx;
    uint32_t t_max_espnowrx;
    uint32_t t_telemetrytx;
    uint32_t t_max_telemetrytx;

    uint8_t cpu_util_percent;

} TelemetryPacket_t;

