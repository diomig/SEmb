/*
 * pwm.c
 *
 *  Created on: 10/01/2023
 *      Author: diomi
 */

#include "main.h"

void PWMInit (void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF0_M1PWM4);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
}


void setSpeaker(int freq){//F2
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, SysCtlClockGet() / freq);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3)) / 2);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
}

void stopSeaker(void){
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);
    //PWMGenDisable(PWM1_BASE, PWM_GEN_3);
}

void setFan(unsigned int duty_cycle){//F0
    int motor_frequency = 500;
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, SysCtlClockGet()/motor_frequency);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) * duty_cycle) / 100);
    PWMOutputState(PWM1_BASE, PWM_OUT_4_BIT, true);
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
}


void stopFan(void){
    PWMOutputState(PWM1_BASE, PWM_OUT_4_BIT, false);
}
