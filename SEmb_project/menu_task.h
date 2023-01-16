/*
 * menu_task.h
 *
 *  Created on: 04/01/2023
 *      Author: diomi
 */

#ifndef MENU_TASK_H_
#define MENU_TASK_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_can.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/buttons.h"
#include "driverlib/can.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "keypad_task.h"
#include "lcd.h"
#include "i2c.h"
#include "actuator_task.h"



#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

uint32_t MenuTaskInit(void);

void date_setup(int* Date);
int time_setup(void);
int timeIsValid(uint8_t key, int time, int position);
void menu(uint8_t key, int* resettime);
float ReadTemp (void);
int getTime();
void showTime(int resetTime, int* date);
void showTemperature();
void showFanSpeed();
uint8_t setMaxTemp(void);
uint8_t setMinTemp(void);

#endif /* MENU_TASK_H_ */
