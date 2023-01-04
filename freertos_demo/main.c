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

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"


#include "led_task.h"
#include "switch_task.h"
#include "keypad_task.h"

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
int date_setup(void);
int time_setup(void);
int8_t getColumn(uint8_t rowbits);
int8_t getRow(uint8_t colbits);
int8_t bitsToIndex(int8_t bits);
void menu(void);

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
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
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
    pinConfiguration();

    lcd_init();
    lcd_clear();
    lcd_put_cur(0);

    //
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    //
    ConfigureUART();

    // Print demo introduction.
    UARTprintf("\n\nWelcome to the EK-TM4C123GXL FreeRTOS Demo!\n");


    // Create a mutex to guard the UART.
    g_pUARTSemaphore = xSemaphoreCreateMutex();

    // Create the LED task.
    if(LEDTaskInit() != 0)
    {
        while(1)
        {
        }
    }


    // Create the switch task.
    if(SwitchTaskInit() != 0)
    {
        while(1)
        {
        }
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
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);


        GPIOIntTypeSet(GPIO_PORTD_BASE, 0x0F, GPIO_RISING_EDGE);
        GPIOIntRegister(GPIO_PORTD_BASE, KeypadInterruptHandler);
        GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);

        //
        // Check if the peripheral access is enabled.
        //
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        {
        }
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        {
        }

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
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xFFFF);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, DebounceHandler);
    TimerEnable(TIMER0_BASE, TIMER_A);

}

int date_setup(void){
    int len = 0;
    int date;
    int position[8] = {6,7,9,10,12,13,14,15};
    int ord_mag[8] = {100000,10000,10000000,100000000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Date: MM/DD/YYYY");
    for(len = 0; len < 8; len++) {
        lcd_put_cur(position[len]);
        while(!key);
        if(key == 'C'){
            len -= 2;
            if(len<0){len=-1;}
        }
        else if(key < 'A'){
            lcd_send_data(key);
            date = (key - '0')*ord_mag[len];
        }
        key=0;
    }
    return date;
}
int time_setup(void){
    int len = 0;
    int time;
    int position[6] = {6,7,9,10,12,13};
    int ord_mag[6] = {100000,10000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Time: hh:mm:ss");
    for(len = 0; len < 6; len++) {
        lcd_put_cur(position[len]);
        while(!key);
        if(key == 'C'){
            len -= 2;
            if(len<0){len=-1;}
        }
        else if(key < 'A'){
            lcd_send_data(key);
            time = (key - '0') * ord_mag[len];
        }
        key=0;
    }
    return time;
}



void menu(void){
    if (key==0) {return;}

    switch(key){
    case 'A':
        lcd_put_cur(0);
        lcd_send_string("Action A");
        break;
    case 'B':
        lcd_put_cur(0);
        lcd_send_string("Action B");
        break;
    case 'C':
        lcd_clear();
        break;
    case 'D':
        lcd_put_cur(0);
        lcd_send_string("Action D");
        break;
    case 'E':
        lcd_put_cur(0);
        lcd_send_string("Action E");
        break;
    case 'F':
        lcd_put_cur(0);
        lcd_send_string("Action F");
        break;
    default:
        lcd_send_data(key);
    }
    key = 0;
}
