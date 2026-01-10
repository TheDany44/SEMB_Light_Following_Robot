#include <Arduino.h>

#include "rtdb.h"
#include "system_mode_manager.h"
#include "motor_control.h"
#include "telemetry_tx.h"
#include "espnow_rx.h"
#include <WiFi.h>
#include <esp_now.h>
#include "ultrasonic_task.h"
#include "ldr_task.h"
#include "calibration_task.h"
#include "light_processing_task.h"
#include "auto_control_task.h"
#include "manual_control_task.h"

uint8_t remoteMAC[] = { 0x88, 0x56, 0xa6, 0x5b, 0x77, 0x1c };

void setup() {
    //pinMode(MOTOR_LEFTA_PWM_PIN, OUTPUT);
    pinMode(MOTOR_LEFTA_IN1_PIN, OUTPUT);
    pinMode(MOTOR_LEFTA_IN2_PIN, OUTPUT);

    //pinMode(MOTOR_RIGHTB_PWM_PIN, OUTPUT);
    pinMode(MOTOR_RIGHTB_IN1_PIN, OUTPUT);
    pinMode(MOTOR_RIGHTB_IN2_PIN, OUTPUT);

    // Force motors OFF
    digitalWrite(MOTOR_LEFTA_IN1_PIN, LOW);
    digitalWrite(MOTOR_LEFTA_IN2_PIN, LOW);
    digitalWrite(MOTOR_RIGHTB_IN1_PIN, LOW);
    digitalWrite(MOTOR_RIGHTB_IN2_PIN, LOW);

    // PWM setup
    ledcSetup(CH_A, PWM_FQ, PWM_RES);
    ledcSetup(CH_B, PWM_FQ, PWM_RES);

    ledcAttachPin(MOTOR_LEFTA_PWM_PIN, CH_A);
    ledcAttachPin(MOTOR_RIGHTB_PWM_PIN, CH_B);

    Serial.begin(115200);
    delay(2000);
    
    rtdb_init();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    // Create command queue before registering callback to avoid ISR using NULL
    cmdQueue = xQueueCreate(10, sizeof(CommandPacket_t));
    if (cmdQueue == NULL) {
        Serial.println("Failed to create cmdQueue");
        return;
    }

    esp_now_register_recv_cb(espNowReceiveCallback);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, remoteMAC, 6);
    peer.channel = 0;
    peer.encrypt = false;

    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println("Failed to add remote peer");
    }

    xTaskCreate(
        systemModeManagerTask,
        "SystemModeManager",
        4096,
        NULL,
        3,      //3
        NULL
    );

    xTaskCreate(
        motorControlTask,
        "MotorControl",
        4096,
        NULL,
        4,      
        NULL
    );

    xTaskCreate(
        telemetryTxTask,
        "TelemetryTX",
        4096,
        NULL,
        1,      // LOW priority
        NULL
    );

    xTaskCreate(
        espNowRxTask,
        "ESPNowRX",
        4096,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        ultrasonicTask,
        "Ultrasonic",
        4096,
        NULL,
        5,   // HIGHEST priority in system
        NULL
    );


    xTaskCreate(
        ldrTask,
        "LDR",
        4096,
        NULL,
        2,   // Medium priority
        NULL
    );

    xTaskCreate(
        calibrationTask,
        "Calibration",
        4096,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        lightProcessingTask,
        "LightProc",
        4096,
        NULL,
        2,   
        NULL
    );

    xTaskCreate(
        autoControlTask,
        "AutoCtrl",
        4096,
        NULL,
        3,   // Medium priority
        NULL
    );

    xTaskCreate(
        manualControlTask,
        "ManualCtrl",
        4096,
        NULL,
        3,   // Same level as AutoCtrl
        NULL
    );


}

void loop() {
    // MUST remain empty
}
