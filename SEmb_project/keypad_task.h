/*
 * keypad_task.h
 *
 *  Created on: 03/01/2023
 *      Author: diomi
 */

#ifndef KEYPAD_TASK_H_
#define KEYPAD_TASK_H_

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
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd.h"

//*****************************************************************************
//
// Prototypes for the keypad task.
//
//*****************************************************************************
extern uint32_t KeypadTaskInit(void);

void KeypadInterruptHandler(void);
int8_t getColumn(uint8_t rowbits);
int8_t getRow(uint8_t colbits);
int8_t bitsToIndex(int8_t bits);
int keyIsValid(uint8_t key);
int  keyIsNumber(uint8_t key);

#endif /* KEYPAD_TASK_H_ */
