#include "ultrasonic_task.h"
#include "rtdb.h"
#include <Arduino.h>


static uint16_t measureDistanceCM() {
    digitalWrite(US_TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(US_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(US_TRIG_PIN, LOW);

    long duration = pulseIn(US_ECHO_PIN, HIGH, 3000); // 3 ms timeout

    if (duration == 0) {
        return 999; // no echo → treat as far
    }

    return duration / 58; // microseconds → cm
}

void ultrasonicTask(void* pvParameters) {

    pinMode(US_TRIG_PIN, OUTPUT);
    pinMode(US_ECHO_PIN, INPUT);

    for (;;) {
        uint32_t t0 = micros(); // start time

        uint16_t dist = measureDistanceCM();

        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "Ultrasonic", 16);

        rtdb_get()->ultrasonicDistance = dist;

        if (dist < OBSTACLE_THRESHOLD_CM) {
            rtdb_get()->obstacleDetected = true;
        } else {
            rtdb_get()->obstacleDetected = false;
        }

        uint32_t exec = micros() - t0;// execution time

        rtdb_get()->timing_ultrasonic.execTime_us = exec; // last execution time
        if (exec > rtdb_get()->timing_ultrasonic.maxTime_us) {
            rtdb_get()->timing_ultrasonic.maxTime_us = exec;
        }//end here
        CPU_ACCUM(exec);

        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(20)); // ~33 Hz
    }
}
