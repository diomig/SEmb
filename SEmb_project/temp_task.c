/*
 * temp_task.c
 *
 *  Created on: 07/01/2023
 *      Author: diomi
 */


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

#define TEMP_ITEM_SIZE sizeof(uint16_t)
#define TEMP_QUEUE_SIZE 10


//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define TEMPTASKSTACKSIZE        1000         // Stack size in words

extern xQueueHandle msgQueue;
//xQueueHandle  TempQueue;
extern xQueueHandle keypadQueue;

xSemaphoreHandle LCDMutex;
xSemaphoreHandle tempMutex;
xSemaphoreHandle fanMutex;

float motorTemp;
uint8_t duty_cycle;
uint8_t MinTEMP = 20;
uint8_t MaxTEMP = 40;

//*****************************************************************************
//
// This task reads the temperature and sends its value
//
//*****************************************************************************


static void TempTask(void *pvParameters) {
    float temp;
    MESSAGE message;
    //bool alarmON = true;

    LCDMutex = xSemaphoreCreateMutex();
    tempMutex = xSemaphoreCreateMutex();
    fanMutex = xSemaphoreCreateMutex();

    I2CInit();

    I2CWriteConf(0x60);

    // Loop forever
    while(1){
        xSemaphoreTake(tempMutex, portMAX_DELAY);
        motorTemp = ReadTemp();
        temp = motorTemp;
        xSemaphoreGive(tempMutex);

        if(checkLimits() == 0){
            duty_cycle = temp_Duty_Cycle(temp);
            message.msg_id = ID_FAN_DUTY_CYCLE;
            message.msg_value = duty_cycle;
            xQueueSend(msgQueue, &message, 0);
            message.msg_id = ID_ALARM_MUTE;
            xQueueSend(msgQueue, &message, 0);
        }
        else if(checkLimits() == -1){
            message.msg_id = ID_FAN_STOP;
            xQueueSend(msgQueue, &message, 0);
            message.msg_id = ID_ALARM_MUTE;
            xQueueSend(msgQueue, &message, 0);
        }
        else if(checkLimits() == 1){
            message.msg_id = ID_FAN_DUTY_CYCLE;
            message.msg_value = 100;
            xQueueSend(msgQueue, &message, 0);
            message.msg_id = ID_ALARM_SET;
            xQueueSend(msgQueue, &message, 0);
        }

        vTaskDelay(500);
    }
}

int checkLimits() {
    xSemaphoreTake(tempMutex, portMAX_DELAY);
    float temp = motorTemp;
    xSemaphoreGive(tempMutex);
    if (temp < MinTEMP){
        return -1;
    }
    else if (temp > MaxTEMP){
        return 1;
    }

    return 0;
}


//************************************************************************************
//************************************************************************************

//*****************************************************************************
//
// Initializes the keypad task.
//
//*****************************************************************************
uint32_t TempTaskInit(void) {
    // Create the temperature control task.
    if(xTaskCreate(TempTask, (const portCHAR *)"Temperature",
                   TEMPTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_TEMP_TASK, NULL) != pdTRUE)
    {
        return(0);
    }


    // Success.
    return(1);
}



uint8_t temp_Duty_Cycle(float temperature){
    uint8_t DCMIN = 10, DC;
    float min = (float)MinTEMP;
    float max = (float)MaxTEMP;

    DC = ((100 - DCMIN)/(max - min)) * temperature + ((- min / max) * 100 + DCMIN)/(1 - (min/max));

    return DC;
}


