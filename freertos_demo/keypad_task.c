/*
 * keypad_task.c
 *
 *  Created on: 03/01/2023
 *      Author: diomi
 */


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/buttons.h"
#include "driverlib/timer.h"
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"

#include "keypad_task.h"

#define KEY_ITEM_SIZE sizeof(uint8_t)
#define KEY_QUEUE_SIZE 1


xSemaphoreHandle keypadSemaphore;
//*****************************************************************************
//*****************************************************************************
const uint8_t keypad[4][4] = {    {'1','2','3','F'},
                                  {'4','5','6','E'},
                                  {'7','8','9','D'},
                                  {'A','0','B','C'}};


//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define KEYPADTASKSTACKSIZE        128         // Stack size in words

xQueueHandle  keypadQueue;

//*****************************************************************************
//
// This task reads the keypad and passes key value to the control unit
//
//*****************************************************************************



void KeypadInterruptHandler(void) {
    static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    int status = GPIOIntStatus(GPIO_PORTD_BASE,true);
    GPIOIntClear(GPIO_PORTD_BASE,status);

    GPIOIntDisable(GPIO_PORTD_BASE, 0x0F);   //temporarily disable interrupts for debounce
    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFF);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);


    /* 'Give' the semaphore to unblock the task. */
    xSemaphoreGiveFromISR( keypadSemaphore, &xHigherPriorityTaskWoken );

    if( xHigherPriorityTaskWoken == pdTRUE ) {
        /* Giving the semaphore unblocked a task, and the priority of the unblocked task
        is higher than the currently running task - force a context keypad to ensure that
        the interrupt returns directly to the unblocked (higher priority) task.
        NOTE: The actual macro to use (context switch) from an ISR is dependent on the
        port. This is the correct macro for the Open Watcom DOS port. Other ports may
        require different syntax */
        portEND_SWITCHING_ISR (xHigherPriorityTaskWoken);
    }
}





static void
KeypadHandlerTask(void *pvParameters)
{
    int8_t column, row;
    uint8_t key;


    // Loop forever
    while(1)
    {
        xSemaphoreTake(keypadSemaphore,portMAX_DELAY);
        column = GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
        if (column != 0)
            row = getRow(column);
        column = bitsToIndex(column);
        row = bitsToIndex(row);
        if (row>=0 && row<=3 && column>=0 && column<=3)
            key = keypad[row][column];

        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);
        row = 0;
        column = 0;
        //GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts

        // Pass the value of the key pressed to the menu task

        if (keyIsValid(key))
            xQueueSend(keypadQueue, &key, 0);

        //GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts
    }
}


int keyIsValid(uint8_t key){
    if (key < '0' || key > 'F')
        return 0;
    return 1;
}

//************************************************************************************
//************************************************************************************

//*****************************************************************************
//
// Initializes the keypad task.
//
//*****************************************************************************
uint32_t
KeypadTaskInit(void)
{
    //
    // Unlock the GPIO LOCK register for Right button to work.
    //
    //HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    //HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0xFF;



    keypadQueue = xQueueCreate(KEY_QUEUE_SIZE, KEY_ITEM_SIZE);

    //keypadSemaphore = xSemaphoreCreateMutex();
    vSemaphoreCreateBinary(keypadSemaphore);
    // Create the keypad task.
    if(xTaskCreate(KeypadHandlerTask, (const portCHAR *)"Keypad",
                   KEYPADTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_KEYPAD_TASK, NULL) != pdTRUE)
    {
        return(0);
    }


    // Success.
    return(1);
}




int8_t getColumn(uint8_t rowbits) {
    GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, rowbits);

    return GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
}

int8_t getRow(uint8_t colbits) {
    int8_t rowbits;
    for(rowbits = 1; rowbits <= 8; rowbits<<=1){
        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, rowbits);
        if(GPIOPinRead(GPIO_PORTD_BASE, 0x0F)) {
            return rowbits;
        }
    }
    return 0;
}

int8_t bitsToIndex(int8_t bits){
    switch(bits) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    default:
        return -1;
    }
}
