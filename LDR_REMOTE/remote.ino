#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

#include "commands.h"
#include "telemetry.h"
#include "web_dashboard.h"

const char* apSSID = "LDR_Robot_Dashboard";
const char* apPASS = "12345678";



// ================= CONFIG =================

//Robot MAC address: 80:65:99:4F:7B:84
uint8_t robotMAC[] = { 0x80, 0x65, 0x99, 0x4F, 0x7B, 0x84 };

// GPIOs
#define BUTTON_STOP_PIN  3
#define BUTTON_CALIB_PIN 21
#define LED_PIN     5

#define JOY_X_PIN     0     // ADC
#define JOY_Y_PIN     1    // ADC
#define JOY_BTN_PIN   2     // Digital (pull-up)

// ==========================================
volatile RobotMode_t robotMode = MODE_INIT;
volatile uint16_t ultrasonicDistance = 0;
volatile bool obstacleDetected = false;

volatile bool robotConnected = false;
volatile uint32_t lastTelemetryTime = 0;
volatile uint16_t ldrRaw[NUM_LDRS];
volatile uint16_t ldrOffset[NUM_LDRS];
volatile int8_t lightDirection = 0;

volatile uint32_t t_ultrasonic;
volatile uint32_t t_max_ultrasonic;
volatile uint32_t t_motor;
volatile uint32_t t_max_motor;
volatile uint32_t t_auto;
volatile uint32_t t_max_auto;
volatile uint32_t t_manual;
volatile uint32_t t_max_manual;
volatile uint32_t t_light;
volatile uint32_t t_max_light;
volatile uint32_t t_ldr;
volatile uint32_t t_max_ldr;
volatile uint32_t t_calibration;
volatile uint32_t t_max_calibration;
volatile uint32_t t_systemmode;
volatile uint32_t t_max_systemmode;
volatile uint32_t t_espnowrx;
volatile uint32_t t_max_espnowrx;
volatile uint32_t t_telemetrytx;
volatile uint32_t t_max_telemetrytx;

volatile uint8_t cpu_util_percent;


static bool lastStopState = LOW;
static bool lastCalibState = LOW;

static bool manualActive = false;
static bool lastJoyBtnState = HIGH;

#define CONNECTION_TIMEOUT_MS 2000  // Consider robot disconnected after 2 seconds
#define JOY_DEADZONE 100            // Joystick deadzone threshold

void onDataRecv(const uint8_t* mac,
                const uint8_t* data,
                int len) {

    if (len == sizeof(TelemetryPacket_t)) {
        TelemetryPacket_t pkt;
        memcpy(&pkt, data, sizeof(pkt));

        robotMode = pkt.currentMode;
        ultrasonicDistance = pkt.ultrasonicDistance;
        obstacleDetected = pkt.obstacleDetected ? true : false;
        lightDirection = pkt.lightDirection;

        for (int i = 0; i < NUM_LDRS; i++) {
            ldrRaw[i] = pkt.ldrRaw[i];
            ldrOffset[i] = pkt.ldrOffset[i];
        }
        t_ultrasonic = pkt.t_ultrasonic;
        t_max_ultrasonic = pkt.t_max_ultrasonic;
        t_motor = pkt.t_motor;
        t_max_motor = pkt.t_max_motor;
        t_auto = pkt.t_auto;
        t_max_auto = pkt.t_max_auto;
        t_manual = pkt.t_manual;
        t_max_manual = pkt.t_max_manual;
        t_light = pkt.t_light;
        t_max_light = pkt.t_max_light;
        t_ldr = pkt.t_ldr;
        t_max_ldr = pkt.t_max_ldr;
        t_calibration = pkt.t_calibration;
        t_max_calibration = pkt.t_max_calibration;
        t_systemmode = pkt.t_systemmode;
        t_max_systemmode = pkt.t_max_systemmode;
        t_espnowrx = pkt.t_espnowrx;
        t_max_espnowrx = pkt.t_max_espnowrx;
        t_telemetrytx = pkt.t_telemetrytx;
        t_max_telemetrytx = pkt.t_max_telemetrytx;
        cpu_util_percent = pkt.cpu_util_percent;


        robotConnected = true;
        lastTelemetryTime = millis();
    }
}


// ESP-NOW send status (optional debug)
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("[ESP-NOW] Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void sendCommand(CommandType_t cmd) {
    CommandPacket_t pkt;
    pkt.cmd = cmd;
    pkt.joyX = 0;
    pkt.joyY = 0;

    esp_now_send(robotMAC, (uint8_t*)&pkt, sizeof(pkt));
}

void sendJoystick(int16_t x, int16_t y) {
    CommandPacket_t pkt;
    pkt.cmd  = CMD_JOYSTICK_DATA;
    pkt.joyX = x;
    pkt.joyY = y;

    esp_now_send(robotMAC, (uint8_t*)&pkt, sizeof(pkt));
}



// -------- LED TASK --------
void ledTask(void* pvParameters) {

    bool ledState = false;
    uint32_t lastToggle = 0;

    const uint32_t SLOW_BLINK_MS = 500;
    const uint32_t FAST_BLINK_MS = 150;

    for (;;) {
        uint32_t now = millis();
        
        // Check connection timeout
        if (robotConnected && (now - lastTelemetryTime) > CONNECTION_TIMEOUT_MS) {
            robotConnected = false;
            manualActive = false;
            Serial.println("[REMOTE] Connection timeout");
        }

        if (!robotConnected) {
            digitalWrite(LED_PIN, LOW);
            ledState = false;
        }
        else if (robotMode == MODE_CALIBRATION) {
            // FAST blink for calibration
            if (now - lastToggle >= FAST_BLINK_MS) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastToggle = now;
            }
        }
        else if (robotMode == MODE_STOPPED ||
                 robotMode == MODE_INIT) {
            // SLOW blink
            if (now - lastToggle >= SLOW_BLINK_MS) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastToggle = now;
            }
        }
        else if (robotMode == MODE_AUTO ||
                 robotMode == MODE_MANUAL) {
            digitalWrite(LED_PIN, HIGH);
            ledState = true;
        }
        if(robotMode == MODE_MANUAL) {
            manualActive = true;
        } else {
            manualActive = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

}

// -------- BUTTON TASK --------
void buttonTask(void* pvParameters) {
    bool state_stop, state_calib;

    for (;;) {
        state_stop = digitalRead(BUTTON_STOP_PIN);
        state_calib = digitalRead(BUTTON_CALIB_PIN);
        bool state = digitalRead(JOY_BTN_PIN);

        // Falling edge detection
        if (lastStopState == HIGH && state_stop == LOW) {
            sendCommand(CMD_START_STOP_TOGGLE);
        }
        if (lastCalibState == HIGH && state_calib == LOW) {
            sendCommand(CMD_CALIBRATE);
        }
        // Button pressed → enable MANUAL
        if (lastJoyBtnState == HIGH && state == LOW) {
            sendCommand(CMD_MANUAL_TOGGLE);
        }

        lastJoyBtnState = state;
        lastStopState = state_stop;
        lastCalibState = state_calib;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void joystickAxisTask(void* pvParameters) {

    for (;;) {
        if (manualActive) {

            int rawX = analogRead(JOY_X_PIN);
            int rawY = analogRead(JOY_Y_PIN);

            int16_t joyX = map(rawX, 0, 4095, -512, 512);
            int16_t joyY = map(rawY, 0, 4095, -512, 512);

            // Deadzone
            if (abs(joyX) < 100) joyX = 0;
            if (abs(joyY) < 100) joyY = 0;

            sendJoystick(joyX, joyY);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("[REMOTE] Starting up...");
    pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_CALIB_PIN, INPUT_PULLUP);
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    lastJoyBtnState = digitalRead(JOY_BTN_PIN);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(onDataRecv);
    esp_now_register_send_cb(onDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, robotMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // ===== SEND CONNECT COMMAND =====
    sendCommand(CMD_CONNECT);
    Serial.println("[REMOTE] CONNECT sent");

    WiFi.softAP(apSSID, apPASS);
    initWebDashboard();

    // Create tasks
    xTaskCreate(
        ledTask,
        "LEDTask",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        buttonTask,
        "ButtonTask",
        2048,
        NULL,
        2,
        NULL
    );
    xTaskCreate(
        joystickAxisTask,
        "JoyAxis",
        2048,
        NULL,
        2,
        NULL
    );
    xTaskCreate(
        webServerTask,
        "WebServer",
        4096,
        NULL,
        1,   // LOWEST priority
        NULL
    );

}

void loop() {
    // Nothing here, everything is handled in tasks
}
