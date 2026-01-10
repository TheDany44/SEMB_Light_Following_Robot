#include "manual_control_task.h"
#include "rtdb.h"
#include <Arduino.h>

#define MAX_SPEED  70

void manualControlTask(void* pvParameters) {

    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "ManualCtrl", 16);

        if (rtdb_get()->currentMode == MODE_MANUAL) {

            // Map joystick: vertical -> forward, horizontal -> turn
            int forward = rtdb_get()->joystickX;
            int turn    = rtdb_get()->joystickY;

            // Differential drive: left = forward + turn, right = forward - turn
            int left  = forward + turn;
            int right = forward - turn;

            left  = constrain(left,  -MAX_SPEED, MAX_SPEED);
            right = constrain(right, -MAX_SPEED, MAX_SPEED);

            rtdb_get()->motorLeftCmd  = left;
            rtdb_get()->motorRightCmd = right;
        }

        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_manual.execTime_us = exec;
        if (exec > rtdb_get()->timing_manual.maxTime_us) {
            rtdb_get()->timing_manual.maxTime_us = exec;
        }
        CPU_ACCUM(exec);

        rtdb_unlock();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
