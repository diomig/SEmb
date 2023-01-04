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
#include "utils/uartstdio.h"
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#include "keypad_task.h"




xSemaphoreHandle keypadSemaphore;
//*****************************************************************************
//*****************************************************************************
const uint8_t keypad[4][4] = {    {'1','2','3','F'},
                                  {'4','5','6','E'},
                                  {'7','8','9','D'},
                                  {'A','0','B','C'}};

volatile uint8_t key = 0;

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define KEYPADTASKSTACKSIZE        128         // Stack size in words

extern xQueueHandle g_pLEDQueue;

//*****************************************************************************
//
// This task reads the keypad and passes key value to the control unit
//
//*****************************************************************************
static void
KeypadHandlerTask(void *pvParameters)
{
    int8_t column, row;

    // Loop forever
    while(1)
    {
        column = GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
        if (column != 0)
            row = getRow(column);
        column = bitsToIndex(column);
        row = bitsToIndex(row);
        if (row>=0 && row<=3 && column>=0 && column<=3)
            key = keypad[row][column];

        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);
    }
}


//************************************************************************************
//************************************************************************************


void KeypadInterruptHandler(void) {
    static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    /* 'Give' the semaphore to unblock the task. */
    xSemaphoreGiveFromISR( keypadSemaphore, &xHigherPriorityTaskWoken );
    if( xHigherPriorityTaskWoken == pdTRUE ) {
        /* Giving the semaphore unblocked a task, and the priority of the unblocked task
        is higher than the currently running task - force a context switch to ensure that
        the interrupt returns directly to the unblocked (higher priority) task.
        NOTE: The actual macro to use (context switch) from an ISR is dependent on the
        port. This is the correct macro for the Open Watcom DOS port. Other ports may
        require different syntax */
        portEND_SWITCHING_ISR (xHigherPriorityTaskWoken);
    }
}
//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
KeypadTaskInit(void)
{
    //
    // Unlock the GPIO LOCK register for Right button to work.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0xFF;

    //
    // Initialize the buttons
    //
    ButtonsInit();


    keypadSemaphore = xSemaphoreCreateMutex();

    //
    // Create the switch task.
    //
    if(xTaskCreate(KeypadHandlerTask, (const portCHAR *)"Switch",
                   KEYPADTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_SWITCH_TASK, NULL) != pdTRUE)
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
