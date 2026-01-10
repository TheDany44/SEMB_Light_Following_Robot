#pragma once
#include <Arduino.h>
#include "commands.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t cmdQueue;


void espNowRxTask(void* pvParameters);
void espNowReceiveCallback(const uint8_t* mac,
                           const uint8_t* data,
                           int len);
