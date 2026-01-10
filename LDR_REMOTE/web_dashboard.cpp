#include "web_dashboard.h"
#include <WiFi.h>
#include <WebServer.h>

#include "telemetry.h"

extern volatile RobotMode_t robotMode;
extern volatile uint16_t ultrasonicDistance;
extern volatile bool obstacleDetected;

extern volatile uint16_t ldrRaw[8];
extern volatile uint16_t ldrOffset[8];
extern volatile int8_t lightDirection;
extern volatile uint32_t t_ultrasonic;
extern volatile uint32_t t_max_ultrasonic;
extern volatile uint32_t t_motor;
extern volatile uint32_t t_max_motor;
extern volatile uint32_t t_auto;
extern volatile uint32_t t_max_auto;
extern volatile uint32_t t_manual;
extern volatile uint32_t t_max_manual;
extern volatile uint32_t t_light;
extern volatile uint32_t t_max_light;
extern volatile uint32_t t_ldr;
extern volatile uint32_t t_max_ldr;
extern volatile uint32_t t_calibration;
extern volatile uint32_t t_max_calibration;
extern volatile uint32_t t_systemmode;
extern volatile uint32_t t_max_systemmode;
extern volatile uint32_t t_espnowrx;
extern volatile uint32_t t_max_espnowrx;
extern volatile uint32_t t_telemetrytx;
extern volatile uint32_t t_max_telemetrytx;

extern volatile uint8_t cpu_util_percent;


WebServer server(80);

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LDR Robot Dashboard</title>
  <style>
    body { font-family: Arial; background: #111; color: #0f0; }
    .box { border: 1px solid #0f0; padding: 10px; margin: 10px; }
  </style>
</head>
<body>
  <h1>LDR Robot Dashboard</h1>

  <div class="box">
    <h2>System</h2>
    Mode: <span id="mode"></span><br>
  </div>

  <div class="box">
    <h2>Safety</h2>
    Ultrasonic: <span id="dist"></span> cm<br>
    Obstacle: <span id="obs"></span>
  </div>

  <div class="box">
    <h2>Light</h2>
    Direction: <span id="dir"></span><br>
    LDR Raw: <span id="ldrRaw"></span><br>
    LDR Offset: <span id="ldrOff"></span>
  </div>
  <div class="box">
    <h2>Task Execution Time (µs)</h2>
    Ultrasonic: <span id="t_us"></span> (max <span id="t_us_max"></span>)<br>
    Motor: <span id="t_motor"></span> (max <span id="t_motor_max"></span>)<br>
    Auto: <span id="t_auto"></span> (max <span id="t_auto_max"></span>)<br>
    Manual: <span id="t_manual"></span> (max <span id="t_manual_max"></span>)<br>
    Light: <span id="t_light"></span> (max <span id="t_light_max"></span>)<br>
    LDR: <span id="t_ldr"></span> (max <span id="t_ldr_max"></span>)<br>
    Calibration: <span id="t_cal"></span> (max <span id="t_cal_max"></span>)<br>
    SystemMode: <span id="t_sm"></span> (max <span id="t_sm_max"></span>)<br>
    ESPNow RX: <span id="t_rx"></span> (max <span id="t_rx_max"></span>)<br>
    Telemetry TX: <span id="t_tx"></span> (max <span id="t_tx_max"></span>)
</div>
<div class="box">
  <h2>CPU Utilization</h2>
  Usage: <span id="cpu"></span> %
</div>



<script>
function update() {
  fetch('/telemetry')
    .then(r => r.json())
    .then(d => {
      document.getElementById('mode').innerText = d.mode;
      document.getElementById('dist').innerText = d.distance;
      document.getElementById('obs').innerText = d.obstacle;
      document.getElementById('dir').innerText = d.lightDir;
      document.getElementById('ldrRaw').innerText = d.ldrRaw;
      document.getElementById('ldrOff').innerText = d.ldrOffset;
      document.getElementById('t_us').innerText = d.t_ultrasonic;
    document.getElementById('t_us_max').innerText = d.t_max_ultrasonic;

    document.getElementById('t_motor').innerText = d.t_motor;
    document.getElementById('t_motor_max').innerText = d.t_max_motor;

    document.getElementById('t_auto').innerText = d.t_auto;
    document.getElementById('t_auto_max').innerText = d.t_max_auto;

    document.getElementById('t_manual').innerText = d.t_manual;
    document.getElementById('t_manual_max').innerText = d.t_max_manual;

    document.getElementById('t_light').innerText = d.t_light;
    document.getElementById('t_light_max').innerText = d.t_max_light;

    document.getElementById('t_ldr').innerText = d.t_ldr;
    document.getElementById('t_ldr_max').innerText = d.t_max_ldr;

    document.getElementById('t_cal').innerText = d.t_calibration;
    document.getElementById('t_cal_max').innerText = d.t_max_calibration;

    document.getElementById('t_sm').innerText = d.t_systemmode;
    document.getElementById('t_sm_max').innerText = d.t_max_systemmode;

    document.getElementById('t_rx').innerText = d.t_espnowrx;
    document.getElementById('t_rx_max').innerText = d.t_max_espnowrx;

    document.getElementById('t_tx').innerText = d.t_telemetrytx;
    document.getElementById('t_tx_max').innerText = d.t_max_telemetrytx;

    document.getElementById("cpu").innerText = d.cpu_util_percent;

    });
}
setInterval(update, 500);
</script>
</body>
</html>
)rawliteral";


void initWebDashboard() {

    server.on("/", []() {
        server.send_P(200, "text/html", DASHBOARD_HTML);
    });

    server.on("/telemetry", []() {
        String json = "{";
        json += "\"mode\":\"" + String(robotMode) + "\",";
        json += "\"distance\":" + String(ultrasonicDistance) + ",";
        json += "\"obstacle\":" + String(obstacleDetected ? "true" : "false") + ",";
        json += "\"lightDir\":" + String(lightDirection) + ",";
        json += "\"ldrRaw\":[";
        for (int i = 0; i < 8; i++) {
            json += String(ldrRaw[i]);
            if (i < 7) json += ",";
        }
        json += "],\"ldrOffset\":[";
        for (int i = 0; i < 8; i++) {
            json += String(ldrOffset[i]);
            if (i < 7) json += ",";
        }
        json += "],";

        // --- Timing ---
        json += "\"t_ultrasonic\":"      + String(t_ultrasonic) + ",";
        json += "\"t_max_ultrasonic\":"  + String(t_max_ultrasonic) + ",";
        json += "\"t_motor\":"           + String(t_motor) + ",";
        json += "\"t_max_motor\":"       + String(t_max_motor) + ",";
        json += "\"t_auto\":"            + String(t_auto) + ",";
        json += "\"t_max_auto\":"        + String(t_max_auto) + ",";
        json += "\"t_manual\":"          + String(t_manual) + ",";
        json += "\"t_max_manual\":"      + String(t_max_manual) + ",";
        json += "\"t_light\":"           + String(t_light) + ",";
        json += "\"t_max_light\":"       + String(t_max_light) + ",";
        json += "\"t_ldr\":"             + String(t_ldr) + ",";
        json += "\"t_max_ldr\":"          + String(t_max_ldr) + ",";
        json += "\"t_calibration\":"     + String(t_calibration) + ",";
        json += "\"t_max_calibration\":" + String(t_max_calibration) + ",";
        json += "\"t_systemmode\":"      + String(t_systemmode) + ",";
        json += "\"t_max_systemmode\":"  + String(t_max_systemmode) + ",";
        json += "\"t_espnowrx\":"        + String(t_espnowrx) + ",";
        json += "\"t_max_espnowrx\":"    + String(t_max_espnowrx) + ",";
        json += "\"t_telemetrytx\":"     + String(t_telemetrytx) + ",";
        json += "\"t_max_telemetrytx\":" + String(t_max_telemetrytx);
        json += ",\"cpu_util_percent\":" + String(cpu_util_percent);

        json += "}";

        server.send(200, "application/json", json);
    });

    server.begin();
}
void webServerTask(void* pvParameters) {
    for (;;) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}