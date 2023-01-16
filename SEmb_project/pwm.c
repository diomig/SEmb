/*
 * pwm.c
 *
 *  Created on: 10/01/2023
 *      Author: diomi
 */

#include "main.h"

void PWMInit (void){
    int freq = 500;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    //PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, SysCtlClockGet() / freq);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
}


void setSpeaker(){//F2
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3)) / 2);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);

}

void stopSeaker(void){
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);
    //PWMGenDisable(PWM1_BASE, PWM_GEN_3);
}

void setFan(unsigned int duty_cycle){//F0
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, (PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3) * duty_cycle) / 100);
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
}


void stopFan(void){
    PWMOutputState(PWM1_BASE, PWM_OUT_4_BIT, false);
}
