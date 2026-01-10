#pragma once
#include <Arduino.h>

//motor pins:
// AIN2 -> frente left
// AIN1 -> back left
// BIN1 -> frente right
// BIN2 -> back right 
// right motor(B) : PWMB -> GPIO13 / IN1 -> GPIO12 / IN2 -> GPIO14
// left motor(A)  : PWMA -> GPIO3 / IN2 -> GPIO5 / IN1 -> GPIO11
//

// LEFT motor (A)
#define MOTOR_LEFTA_PWM_PIN   3     // PWMA
#define MOTOR_LEFTA_IN1_PIN   11    // AIN1 -> back left
#define MOTOR_LEFTA_IN2_PIN   5     // AIN2 -> front left

// RIGHT motor (B)
#define MOTOR_RIGHTB_PWM_PIN   13    // PWMB
#define MOTOR_RIGHTB_IN1_PIN   14    // BIN1 -> front right
#define MOTOR_RIGHTB_IN2_PIN   12    // BIN2 -> back right

const int CH_A   = 0;       // PWM channel for Motor A
const int CH_B   = 1;       // PWM channel for Motor B
const int PWM_FQ = 10000;   // 10 kHz
const int PWM_RES = 8;      // 8-bit resolution (0–255)

void motorControlTask(void* pvParameters);
