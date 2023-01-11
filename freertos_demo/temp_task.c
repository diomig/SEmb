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

xQueueHandle  TempQueue;
extern xQueueHandle keypadQueue;

xSemaphoreHandle LCDMutex;
xSemaphoreHandle tempMutex;

float motorTemp;
uint8_t MinTEMP = 20;
uint8_t MaxTEMP = 40;

//*****************************************************************************
//
// This task reads the temperature and sends its value
//
//*****************************************************************************


static void TempTask(void *pvParameters) {
    //uint16_t temp;
    MESSAGE message;
    bool alarmON = true;

    LCDMutex = xSemaphoreCreateMutex();
    tempMutex = xSemaphoreCreateMutex();

    I2CInit();

    I2CWriteConf(0x60);

    // Loop forever
    while(1){


        //temp = I2CRead(REG_TEMP_ADDR);
        xSemaphoreTake(tempMutex, portMAX_DELAY);
        motorTemp = ReadTemp();
        xSemaphoreGive(tempMutex);

        //setFan(MaxTEMP);
        message.msg_id = ID_MOTOR_DUTY_CYCLE;
        message.msg_value = MaxTEMP
        xQueueSend(msgQueue, )

//        checkLimits(&alarmON);


        vTaskDelay(500);


        //xQueueSend(TempQueue, &temp, 0);
    }
}

int checkLimits(bool* alarmON) {
    uint8_t key;

    char alarm_msg[20];
    xSemaphoreTake(tempMutex, portMAX_DELAY);
    float temp = motorTemp;
    xSemaphoreGive(tempMutex);
    if (motorTemp < MinTEMP){

        sprintf(alarm_msg, "Below min (%.2f)", temp);
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_clear();
        lcd_send_string(alarm_msg);
//        lcd_send_string("Below min");
        xSemaphoreGive(LCDMutex);
    }
    else if (motorTemp > MaxTEMP){

        sprintf(alarm_msg, "Above max (%.2f)", temp);
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_clear();

        lcd_send_string(alarm_msg);
//        lcd_send_string("Above max");


        if(*alarmON)
            setSpeaker(500);


        xQueueReceive(keypadQueue, &key, 2000);
        if(keyIsValid(key)) {
            *alarmON = !(*alarmON);
        }

        stopSeaker();
        lcd_clear();

        xSemaphoreGive(LCDMutex);


        //vTaskDelay(1000);

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
