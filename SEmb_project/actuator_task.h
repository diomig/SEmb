/*
 * actuator_task.h
 *
 *  Created on: 11/01/2023
 *      Author: diomi
 */

#ifndef ACTUATOR_TASK_H_
#define ACTUATOR_TASK_H_

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
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "keypad_task.h"
#include "menu_task.h"
#include "lcd.h"
#include "i2c.h"
#include "pwm.h"


#define ID_START            0
#define ID_FAN_DUTY_CYCLE   1
#define ID_FAN_STOP         2
#define ID_ALARM_SET        3
#define ID_ALARM_MUTE       4


typedef struct message{
    int msg_id;
    int msg_value;
}MESSAGE;

uint32_t ActuatorTaskInit(void);

#endif /* ACTUATOR_TASK_H_ */
