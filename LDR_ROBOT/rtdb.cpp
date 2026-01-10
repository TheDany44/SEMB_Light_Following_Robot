#include "rtdb.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "commands.h"

static RobotRTDB_t rtdb;
static SemaphoreHandle_t rtdbMutex;

void rtdb_init(void) {
    rtdbMutex = xSemaphoreCreateMutex();

    rtdb.currentMode = MODE_INIT;
    rtdb.obstacleDetected = false;
    rtdb.ultrasonicDistance = 0;
    rtdb.manualOverrideActive = false;
    rtdb.motorLeftCmd  = 0;
    rtdb.motorRightCmd = 0;
    rtdb.lastCommand = CMD_NONE;
    rtdb.lightDirection = -1;
    rtdb.joystickX = 0;
    rtdb.joystickY = 0;

    rtdb.timing_ultrasonic = {0, 0}; //
    rtdb.timing_motor      = {0, 0}; //
    rtdb.timing_auto       = {0, 0}; //
    rtdb.timing_manual     = {0, 0}; //
    rtdb.timing_lightproc  = {0, 0}; //
    rtdb.timing_ldr        = {0, 0}; //
    rtdb.timing_calibration= {0, 0}; //
    rtdb.timing_systemmode = {0, 0}; //
    rtdb.timing_espnowrx   = {0, 0}; //
    rtdb.timing_telemetrytx= {0, 0}; //
    rtdb.cpu_exec_accum_us = 0;
    rtdb.cpu_window_start_us = micros();
    rtdb.cpu_util_percent = 0;



    for (int i = 0; i < NUM_LDRS; i++) {
        rtdb.ldrRaw[i] = 0;
        rtdb.ldrOffset[i] = 0;
        rtdb.ldrCorrected[i] = 0;
    }

    strncpy(rtdb.activeTaskName, "INIT", 16);
}

void rtdb_lock(void) {
    xSemaphoreTake(rtdbMutex, portMAX_DELAY);
}

void rtdb_unlock(void) {
    xSemaphoreGive(rtdbMutex);
}

RobotRTDB_t* rtdb_get(void) {
    return &rtdb;
}
