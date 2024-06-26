//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include "main.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>FreeRTOS Example (freertos_demo)</h1>
//!
//! This application demonstrates the use of FreeRTOS on Launchpad.
//!
//! The application blinks the user-selected LED at a user-selected frequency.
//! To select the LED press the left button and to select the frequency press
//! the right button.  The UART outputs the application status at 115,200 baud,
//! 8-n-1 mode.
//!
//! This application utilizes FreeRTOS to perform the tasks in a concurrent
//! fashion.  The following tasks are created:
//!
//! - An LED task, which blinks the user-selected on-board LED at a
//!   user-selected rate (changed via the buttons).
//!
//! - A Switch task, which monitors the buttons pressed and passes the
//!   information to LED task.
//!
//! In addition to the tasks, this application also uses the following FreeRTOS
//! resources:
//!
//! - A Queue to enable information transfer between tasks.
//!
//! - A Semaphore to guard the resource, UART, from access by multiple tasks at
//!   the same time.
//!
//! - A non-blocking FreeRTOS Delay to put the tasks in blocked state when they
//!   have nothing to do.
//!
//! For additional details on FreeRTOS, refer to the FreeRTOS web page at:
//! http://www.freertos.org/
//
//*****************************************************************************


void keypadHandler();
void DebounceHandler();

void pinConfiguration(void);
void timerConfiguration(void);


void PWMInit (void);

//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}



//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    // Set the clocking to run at 50 MHz from the PLL.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);


    timerConfiguration();
    pinConfiguration();     //configuracoes dos pins do Tiva

    lcd_init();     //inicializacao do LCD
    lcd_clear();
    lcd_put_cur(0);
    PWMInit();      //inicializacao do modulo PWM



    // Create the temperature control task.
    if(!TempTaskInit()) {
        while(1) {}
    }

    // Create the Keypad task.
    if(!KeypadTaskInit()) {
        while(1) {}
    }

    // Create the menu task.
    if(!MenuTaskInit()) {
        while(1) {}
    }

    // Create the actuator task.
    if(!ActuatorTaskInit()) {
        while(1) {}
    }

    // Start the scheduler.  This should not return.
    vTaskStartScheduler();


    // In case the scheduler returns for some reason, print an error and loop forever.
    while(1)
    {
    }
}





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Function Declarations
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void pinConfiguration(void) {
    //
        // Enable the GPIO port that is used for the on-board LED.
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

        GPIOIntTypeSet(GPIO_PORTD_BASE, 0x0F, GPIO_RISING_EDGE);
        GPIOIntRegister(GPIO_PORTD_BASE, KeypadInterruptHandler);
        GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);

        //
        // Check if the peripheral access is enabled.
        //
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

        GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3);
        GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, 0xF0);
        GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, 0x0F);
        GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x0F);

        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);

}


void timerConfiguration(void) {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFF);

    TimerIntRegister(TIMER0_BASE, TIMER_A, DebounceHandler);
    TimerEnable(TIMER0_BASE, TIMER_A);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


void DebounceHandler(){
    //TimerIntClear(TIMER0_BASE, TIMER_A);
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts
}




