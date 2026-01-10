#pragma once
#include <Arduino.h>
#include <stdbool.h>
#include "commands.h"
#define NUM_LDRS 8
#define CALIBRATION_SAMPLES 50

#define CPU_ACCUM(exec_us) \
    rtdb_get()->cpu_exec_accum_us += (exec_us)

typedef enum {
    MODE_INIT,
    MODE_STOPPED,
    MODE_CALIBRATION,
    MODE_AUTO,
    MODE_MANUAL
} RobotMode_t;

typedef struct {
    uint32_t execTime_us;   // last execution time
    uint32_t maxTime_us;    // worst-case observed
} TaskTiming_t;

typedef struct {
    RobotMode_t currentMode;

    bool obstacleDetected;
    uint16_t ultrasonicDistance;

    //LDR Sensing
    uint16_t ldrRaw[NUM_LDRS];
    uint16_t ldrOffset[NUM_LDRS];
    int16_t  ldrCorrected[NUM_LDRS];

    // Perception
    int8_t lightDirection;   // -1 = none, 0..7 = LDR index

    bool manualOverrideActive; //not implemented yet

    // Motor command abstraction
    int motorLeftCmd;   // -255 .. +255
    int motorRightCmd;  // -255 .. +255


    // Manual control (robot side)
    int16_t joystickX;
    int16_t joystickY;


    CommandType_t lastCommand;

    TaskTiming_t timing_ultrasonic; // ultrasonic sensing
    TaskTiming_t timing_motor; // motor control
    TaskTiming_t timing_auto; // auto control
    TaskTiming_t timing_manual; // manual control
    TaskTiming_t timing_lightproc; // light processing
    TaskTiming_t timing_ldr; // ldr sensing
    TaskTiming_t timing_calibration; // calibration
    TaskTiming_t timing_systemmode; // system mode manager
    TaskTiming_t timing_espnowrx; // esp-now reception
    TaskTiming_t timing_telemetrytx; // telemetry transmission

    uint32_t cpu_exec_accum_us;   // sum of all task exec times in window
    uint32_t cpu_window_start_us; // window start time
    uint8_t  cpu_util_percent;   // computed utilization

    char activeTaskName[16];
} RobotRTDB_t;

void rtdb_init(void);
void rtdb_lock(void);
void rtdb_unlock(void);
RobotRTDB_t* rtdb_get(void);


