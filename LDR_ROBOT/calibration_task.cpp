#include "calibration_task.h"
#include "rtdb.h"
#include <Arduino.h>

void calibrationTask(void* pvParameters) {
    static bool calibrated = false;

    for (;;) {
        uint32_t t0 = micros(); // start time
        uint32_t pauseExec = 0;
        uint32_t mini = 0;
        uint32_t mini_aux = 0;
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "Calib", 16);

        if (rtdb_get()->currentMode == MODE_CALIBRATION && !calibrated) {

            uint32_t sum[NUM_LDRS] = {0};

            pauseExec = micros() - t0;// execution time
            CPU_ACCUM(pauseExec);
            // Release lock while sampling
            rtdb_unlock();

            for (int s = 0; s < CALIBRATION_SAMPLES; s++) {
                mini = micros();
                rtdb_lock();
                for (int i = 0; i < NUM_LDRS; i++) {
                    sum[i] += rtdb_get()->ldrRaw[i];
                }
                rtdb_unlock();
                mini_aux = micros() - mini;
                if(s==0)
                {
                    mini_aux += pauseExec;
                }

                rtdb_lock();
                rtdb_get()->timing_calibration.execTime_us = mini_aux;
                if (mini_aux > rtdb_get()->timing_calibration.maxTime_us) {
                    rtdb_get()->timing_calibration.maxTime_us = mini_aux;
                }
        
                CPU_ACCUM(mini_aux);
                rtdb_unlock();
                vTaskDelay(pdMS_TO_TICKS(75));
            }
            
            t0 = micros();
            rtdb_lock();
            for (int i = 0; i < NUM_LDRS; i++) {
                rtdb_get()->ldrOffset[i] = sum[i] / CALIBRATION_SAMPLES;
            }

            rtdb_get()->currentMode = MODE_STOPPED;
            calibrated = true;
            rtdb_unlock();
        }
        else if (rtdb_get()->currentMode != MODE_CALIBRATION) {
            calibrated = false;
            rtdb_unlock();
        }
        else {
            rtdb_unlock();
        }
        uint32_t exec = micros() - t0;// execution time
        
        rtdb_lock();
        CPU_ACCUM(exec);
        rtdb_get()->timing_calibration.execTime_us = exec;
        if (exec > rtdb_get()->timing_calibration.maxTime_us) {
            rtdb_get()->timing_calibration.maxTime_us = exec;
        }
        rtdb_unlock();
        vTaskDelay(pdMS_TO_TICKS(75));
    }
}
