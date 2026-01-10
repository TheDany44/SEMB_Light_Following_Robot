#include "auto_control_task.h"
#include "rtdb.h"
#include <Arduino.h>

#define BASE_SPEED     100
#define TURN_SPEED  BASE_SPEED / 2
#define STOP_SPEED       0

void autoControlTask(void* pvParameters) {

    for (;;) {
        uint32_t t0 = micros(); // start time

        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "AutoCtrl", 16);

        if (rtdb_get()->currentMode == MODE_AUTO) {

            int8_t dir = rtdb_get()->lightDirection;

            if (dir < 0) {
                // No significant light → stop
                rtdb_get()->motorLeftCmd  = STOP_SPEED;
                rtdb_get()->motorRightCmd = STOP_SPEED;
            }
            else {
                switch (dir) {
                    case 4: // front
                        rtdb_get()->motorLeftCmd  = BASE_SPEED;
                        rtdb_get()->motorRightCmd = BASE_SPEED;
                        break;

                    case 3: //front and right
                        rtdb_get()->motorLeftCmd  = BASE_SPEED;
                        rtdb_get()->motorRightCmd = 4*TURN_SPEED/3;
                        break;
                    case 2: // right
                        rtdb_get()->motorLeftCmd  = BASE_SPEED;
                        rtdb_get()->motorRightCmd = TURN_SPEED;
                        break;

                    case 1: // back and right
                        rtdb_get()->motorLeftCmd  = BASE_SPEED;
                        rtdb_get()->motorRightCmd = 2*TURN_SPEED/3;
                        break;


                    case 0: // back → rotate
                        rtdb_get()->motorLeftCmd  = BASE_SPEED;
                        rtdb_get()->motorRightCmd = -BASE_SPEED;
                        break;

                    case 7: // back and left
                        rtdb_get()->motorLeftCmd  = 2*TURN_SPEED/3;
                        rtdb_get()->motorRightCmd = BASE_SPEED;
                        break;
                    case 6: // left
                        rtdb_get()->motorLeftCmd  = TURN_SPEED;
                        rtdb_get()->motorRightCmd = BASE_SPEED;
                        break;

                    case 5: // front and left
                        rtdb_get()->motorLeftCmd  = 4*TURN_SPEED/3;
                        rtdb_get()->motorRightCmd = BASE_SPEED;
                        break;
                }
            }
        }

        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_auto.execTime_us = exec;
        if (exec > rtdb_get()->timing_auto.maxTime_us) {
            rtdb_get()->timing_auto.maxTime_us = exec;
        }
        CPU_ACCUM(exec);

        rtdb_unlock();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
