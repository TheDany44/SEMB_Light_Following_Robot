#include "telemetry_tx.h"
#include "rtdb.h"
#include <WiFi.h>
#include <esp_now.h>

void telemetryTxTask(void* pvParameters) {

    TelemetryPacket_t pkt;

    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();

        uint32_t now = micros();
        uint32_t window = now - rtdb_get()->cpu_window_start_us;

        if (window >= 1000000) { // 1 second

            uint32_t util =
                (rtdb_get()->cpu_exec_accum_us * 100) / window;

            if (util > 100) util = 100;

            rtdb_get()->cpu_util_percent = util;

            // reset window
            rtdb_get()->cpu_exec_accum_us = 0;
            rtdb_get()->cpu_window_start_us = now;
        }

        pkt.currentMode = rtdb_get()->currentMode;
        pkt.ultrasonicDistance = rtdb_get()->ultrasonicDistance;
        pkt.obstacleDetected = rtdb_get()->obstacleDetected ? 1 : 0;
        pkt.lightDirection = rtdb_get()->lightDirection;
        for (int i = 0; i < NUM_LDRS; i++) {
            pkt.ldrRaw[i] = rtdb_get()->ldrRaw[i];
            pkt.ldrOffset[i] = rtdb_get()->ldrOffset[i];
        }
        pkt.t_ultrasonic    = rtdb_get()->timing_ultrasonic.execTime_us;
        pkt.t_max_ultrasonic= rtdb_get()->timing_ultrasonic.maxTime_us;
        pkt.t_motor         = rtdb_get()->timing_motor.execTime_us;
        pkt.t_max_motor     = rtdb_get()->timing_motor.maxTime_us;
        pkt.t_auto          = rtdb_get()->timing_auto.execTime_us;
        pkt.t_max_auto      = rtdb_get()->timing_auto.maxTime_us;
        pkt.t_manual        = rtdb_get()->timing_manual.execTime_us;
        pkt.t_max_manual    = rtdb_get()->timing_manual.maxTime_us;
        pkt.t_light         = rtdb_get()->timing_lightproc.execTime_us;
        pkt.t_max_light     = rtdb_get()->timing_lightproc.maxTime_us;
        pkt.t_ldr           = rtdb_get()->timing_ldr.execTime_us;
        pkt.t_max_ldr       = rtdb_get()->timing_ldr.maxTime_us;
        pkt.t_calibration   = rtdb_get()->timing_calibration.execTime_us;
        pkt.t_max_calibration= rtdb_get()->timing_calibration.maxTime_us;
        pkt.t_systemmode    = rtdb_get()->timing_systemmode.execTime_us;
        pkt.t_max_systemmode= rtdb_get()->timing_systemmode.maxTime_us;
        pkt.t_espnowrx      = rtdb_get()->timing_espnowrx.execTime_us;
        pkt.t_max_espnowrx  = rtdb_get()->timing_espnowrx.maxTime_us;
        pkt.t_telemetrytx   = rtdb_get()->timing_telemetrytx.execTime_us;
        pkt.t_max_telemetrytx= rtdb_get()->timing_telemetrytx.maxTime_us;
        pkt.cpu_util_percent = rtdb_get()->cpu_util_percent;

        /*Serial.print("Telemetry Packet:");
        Serial.print(" LightDir: "); Serial.print(pkt.lightDirection);
        for (int i = 0; i < NUM_LDRS; i++) {
            Serial.print(" LDR"); Serial.print(i);
            Serial.print(":"); Serial.print(pkt.ldrRaw[i]);
            Serial.print("/");  
        }*/

        //Serial.print(" Mode: "); Serial.print(pkt.currentMode);
        //Serial.print(" Last Command: "); Serial.println(rtdb_get()->lastCommand);
        /*Serial.print(" Distance: "); Serial.print(pkt.ultrasonicDistance);
        Serial.print(" Obstacle: "); Serial.print(pkt.obstacleDetected);
        Serial.print(" LightDir: "); Serial.print(pkt.lightDirection);
        Serial.print(" LDR Raw/Offset:");
        for (int i = 0; i < NUM_LDRS; i++) {
            pkt.ldrRaw[i] = rtdb_get()->ldrRaw[i];
            pkt.ldrOffset[i] = rtdb_get()->ldrOffset[i];
            Serial.print(i);
            Serial.print(pkt.ldrRaw[i]);
            Serial.print("/");
            Serial.print(pkt.ldrOffset[i]);
            Serial.print(" ");
        }
        Serial.println();*/
        //Serial.println();
        rtdb_unlock();

        esp_now_send(remoteMAC,
                     (uint8_t*)&pkt,
                     sizeof(pkt));

        rtdb_lock();
        uint32_t exec = micros() - t0;// execution time
        rtdb_get()->timing_telemetrytx.execTime_us = exec;
        if (exec > rtdb_get()->timing_telemetrytx.maxTime_us) {
            rtdb_get()->timing_telemetrytx.maxTime_us = exec;
        }
        CPU_ACCUM(exec);
        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}



/*static const char* modeToString(RobotMode_t mode) {
    switch (mode) {
        case MODE_INIT: return "INIT";
        case MODE_STOPPED: return "STOPPED";
        case MODE_CALIBRATION: return "CALIBRATION";
        case MODE_AUTO: return "AUTO";
        case MODE_MANUAL: return "MANUAL";
        default: return "UNKNOWN";
    }
}

void telemetryTxTask(void* pvParameters) {
    for (;;) {
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "Telemetry", 16);

        Serial.print("[Telemetry] Mode: ");
        Serial.print(modeToString(rtdb_get()->currentMode));
        Serial.print(" | ActiveTask: ");
        Serial.println(rtdb_get()->activeTaskName);

        rtdb_unlock();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}*/
