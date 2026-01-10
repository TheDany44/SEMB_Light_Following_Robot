#include "motor_control.h"
#include "rtdb.h"

static void setMotorA(int cmd) {

    if (cmd>255)
        cmd = 255;
    if (cmd<-255)
        cmd = -255;

    //cmd = constrain(cmd, -255, 255);

    if (cmd > 0) {
        digitalWrite(MOTOR_LEFTA_IN1_PIN, LOW);
        digitalWrite(MOTOR_LEFTA_IN2_PIN, HIGH);
        //analogWrite(MOTOR_LEFTA_PWM_PIN, cmd);
    } else if (cmd < 0) {
        digitalWrite(MOTOR_LEFTA_IN1_PIN, HIGH);
        digitalWrite(MOTOR_LEFTA_IN2_PIN, LOW);
        //analogWrite(MOTOR_LEFTA_PWM_PIN, -cmd);
    } else {
        digitalWrite(MOTOR_LEFTA_IN1_PIN, LOW);
        digitalWrite(MOTOR_LEFTA_IN2_PIN, LOW);
        //analogWrite(MOTOR_LEFTA_PWM_PIN, 0);
    }
    int pwmVal = abs(cmd);
    ledcWrite(CH_A, pwmVal);
}

static void setMotorB(int cmd) {
     if (cmd>255)
        cmd = 255;
    if (cmd<-255)
        cmd = -255;

    if (cmd > 0) {
        digitalWrite(MOTOR_RIGHTB_IN1_PIN, HIGH);
        digitalWrite(MOTOR_RIGHTB_IN2_PIN, LOW);
        //analogWrite(MOTOR_RIGHTB_PWM_PIN, cmd);
    } else if (cmd < 0) {
        digitalWrite(MOTOR_RIGHTB_IN1_PIN, LOW);
        digitalWrite(MOTOR_RIGHTB_IN2_PIN, HIGH);
        //analogWrite(MOTOR_RIGHTB_PWM_PIN, -cmd);
    } else {
        digitalWrite(MOTOR_RIGHTB_IN1_PIN, LOW);
        digitalWrite(MOTOR_RIGHTB_IN2_PIN, LOW);
        //analogWrite(MOTOR_RIGHTB_PWM_PIN, 0);
    }
    ledcWrite(CH_B, abs(cmd));
}


void motorControlTask(void* pvParameters) {

    for (;;) {
        uint32_t t0 = micros(); // start time
        rtdb_lock();
        strncpy(rtdb_get()->activeTaskName, "MotorCtrl", 16);

        RobotMode_t mode = rtdb_get()->currentMode;
        int leftCmd  = rtdb_get()->motorLeftCmd;
        int rightCmd = rtdb_get()->motorRightCmd;

        rtdb_unlock();
        // ENFORCEMENT LAYER
        if (mode == MODE_AUTO || mode == MODE_MANUAL) {
            setMotorA(leftCmd);
            setMotorB(rightCmd);
        } else {
            // Hard safety stop
            setMotorA(0);
            setMotorB(0);
        }

        rtdb_lock();
        uint32_t exec = micros() - t0;// execution time

        rtdb_get()->timing_motor.execTime_us = exec; // last execution time
        if (exec > rtdb_get()->timing_motor.maxTime_us) {
            rtdb_get()->timing_motor.maxTime_us = exec;
        }//end here
        CPU_ACCUM(exec);

        rtdb_unlock();

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
