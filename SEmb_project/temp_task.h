/*
 * temp_task.h
 *
 *  Created on: 07/01/2023
 *      Author: diomi
 */

#ifndef TEMP_TASK_H_
#define TEMP_TASK_H_


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
#include "menu_task.h"
#include "lcd.h"
#include "i2c.h"
#include "keypad_task.h"
#include "temp_task.h"
#include "pwm.h"
#include "actuator_task.h"


extern uint32_t TempTaskInit(void);


int checkLimits();
uint8_t temp_Duty_Cycle(float temperature);

#endif /* TEMP_TASK_H_ */
