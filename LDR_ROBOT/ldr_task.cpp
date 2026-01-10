#include "ldr_task.h"
#include "rtdb.h"
#include <Arduino.h>

//LDRS
//LDR5-> GPIO7
//LDR7 -> GPIO9
//LDR1 -> GPIO1
//LDR2 -> GPIO2
//LDR4 -> GPIO4
//LDR6 -> GPIO6
//LDR8 -> GPIO8
//LDR10 -> GPIO10

//     17
//  18    4
// 5       10
//   7   6
//    8

//---------------------------------
//      4
//   5     3
// 6          2
//   7     1  
//      0

static const uint8_t ldrPins[NUM_LDRS] = {
    8, 6, 10, 4, 17, 18, 7, 9  
};

void ldrTask(void* pvParameters) {

    for (int i = 0; i < NUM_LDRS; i++) {
        pinMode(ldrPins[i], INPUT);
    }

    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "LDR", 16);

        for (int i = 0; i < NUM_LDRS; i++) {
            rtdb_get()->ldrRaw[i] = analogRead(ldrPins[i]);
        }
        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_ldr.execTime_us = exec;
        if (exec > rtdb_get()->timing_ldr.maxTime_us) {
            rtdb_get()->timing_ldr.maxTime_us = exec;
        }
        CPU_ACCUM(exec);

        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(75)); // ~13 Hz
    }
}
