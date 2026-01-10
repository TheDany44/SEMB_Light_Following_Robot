#include "light_processing_task.h"
#include "rtdb.h"
#include <Arduino.h>

#define NUM_LDRS 8
#define LIGHT_THRESHOLD  500   // ignore noise / darkness

void lightProcessingTask(void* pvParameters) {

    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "LightProc", 16);

        int maxValue = 0;
        int maxIndex = -1;

        for (int i = 0; i < NUM_LDRS; i++) {
            int16_t corrected =
                (int16_t)rtdb_get()->ldrRaw[i] -
                (int16_t)rtdb_get()->ldrOffset[i];

            if (corrected < 0) {
                corrected = 0;
            }

            rtdb_get()->ldrCorrected[i] = corrected;

            if (corrected > maxValue) {
                maxValue = corrected;
                maxIndex = i;
            }
        }

        // Only accept a direction if light is strong enough
        if (maxValue > LIGHT_THRESHOLD) {
            rtdb_get()->lightDirection = maxIndex;
        } else {
            rtdb_get()->lightDirection = -1; // no significant light
        }

        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_lightproc.execTime_us = exec;
        if (exec > rtdb_get()->timing_lightproc.maxTime_us) {
            rtdb_get()->timing_lightproc.maxTime_us = exec;
        }
        CPU_ACCUM(exec);

        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(75));
    }
}
