#include "system_mode_manager.h"
#include "rtdb.h"
#include "commands.h"

void systemModeManagerTask(void* pvParameters) {
    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();
        RobotMode_t mode = rtdb_get()->currentMode;
        CommandType_t cmd = rtdb_get()->lastCommand;

        // --- MODE TRANSITION LOGIC (PHASE 3) ---

        switch (mode) {

            case MODE_INIT:
                if (cmd == CMD_CONNECT) {
                    rtdb_get()->currentMode = MODE_STOPPED;
                }
                break;

            case MODE_STOPPED:
                if (cmd == CMD_START_STOP_TOGGLE) {
                    rtdb_get()->currentMode = MODE_AUTO;
                }
                else if (cmd == CMD_CALIBRATE) {
                    rtdb_get()->currentMode = MODE_CALIBRATION;
                }
                else if (cmd == CMD_MANUAL_TOGGLE) {
                    rtdb_get()->currentMode = MODE_MANUAL;
                }
                break;

            case MODE_AUTO:
                if (cmd == CMD_START_STOP_TOGGLE || rtdb_get()->obstacleDetected==true) {
                    rtdb_get()->currentMode = MODE_STOPPED;
                }
                else if (cmd == CMD_MANUAL_TOGGLE) {
                    rtdb_get()->currentMode = MODE_MANUAL;
                }
                break;
            case MODE_MANUAL:
                if (cmd == CMD_START_STOP_TOGGLE) {
                    rtdb_get()->currentMode = MODE_STOPPED;
                }
                else if (cmd == CMD_MANUAL_TOGGLE) {
                    rtdb_get()->currentMode = MODE_AUTO;
                }
                
                break;

            default:
                // Should never happen
                break;
        }

        // --- COMMAND CONSUMED ---
        // Important: prevent re-triggering
        rtdb_get()->lastCommand = CMD_NONE;

        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_systemmode.execTime_us = exec;
        if (exec > rtdb_get()->timing_systemmode.maxTime_us) {
            rtdb_get()->timing_systemmode.maxTime_us = exec;
        }
        CPU_ACCUM(exec);

        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


