#pragma once

#define US_TRIG_PIN  33
#define US_ECHO_PIN  39

#define OBSTACLE_THRESHOLD_CM  20   // stop if closer than this


void ultrasonicTask(void* pvParameters);


