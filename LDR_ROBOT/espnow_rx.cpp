#include <esp_now.h>
#include "espnow_rx.h"
#include "rtdb.h"

QueueHandle_t cmdQueue;

void espNowReceiveCallback(const uint8_t* mac,
                            const uint8_t* data,
                            int len) {
    if (len == sizeof(CommandPacket_t)) {
        CommandPacket_t pkt;
        memcpy(&pkt, data, sizeof(pkt));

        if (cmdQueue != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(cmdQueue, &pkt, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

void espNowRxTask(void* pvParameters) {
    CommandPacket_t pkt;

    for (;;) {
        if (xQueueReceive(cmdQueue, &pkt, portMAX_DELAY)) {
            uint32_t t0 = micros(); // start time
            // Forward to Mode Manager via shared state
            rtdb_lock();
            strncpy(rtdb_get()->activeTaskName, "ESPNowRX", 16);

            // Store last command (Phase 3 minimal handling)
            rtdb_get()->lastCommand = pkt.cmd;
            Serial.print("[ESPNowRX] Received command: ");
            Serial.println(pkt.cmd);
            if (pkt.cmd == CMD_JOYSTICK_DATA) {
                rtdb_get()->joystickX = pkt.joyX;
                rtdb_get()->joystickY = pkt.joyY;
            }
            uint32_t exec = micros() - t0;// execution time
            rtdb_get()->timing_espnowrx.execTime_us = exec;
            if (exec > rtdb_get()->timing_espnowrx.maxTime_us) {
                rtdb_get()->timing_espnowrx.maxTime_us = exec;
            }
            CPU_ACCUM(exec);


            rtdb_unlock();
        }
    }
}
