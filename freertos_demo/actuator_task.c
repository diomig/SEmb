/*
 * actuator_task.c
 *
 *  Created on: 11/01/2023
 *      Author: diomi
 */



#include "actuator_task.h"

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define ACTUATORTASKSTACKSIZE        1000         // Stack size in words

#define MSG_QUEUE_SIZE 10
#define MSG_ITEM_SIZE sizeof(MESSAGE)
xQueueHandle msgQueue;

MESSAGE message;

//*****************************************************************************
//
// This task reads the buttons' state and passes this information to LEDTask.
//
//*****************************************************************************
static void
ActuatorTask(void *pvParameters)
{

    msgQueue = xQueueCreate(MSG_QUEUE_SIZE, MSG_ITEM_SIZE);
    // Loop forever.
    while(1)
    {

        xQueueReceive(msgQueue, &message, portMAX_DELAY);
        if(message.msg_id == ID_MOTOR_DUTY_CYCLE) {
            setFan(message.msg_value);
        }
        else if(message.msg_id == ID_ALARM_FREQUENCY){
            setSpeaker(message.msg_value);
        }
        else
            while(1);

    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t ActuatorTaskInit(void) {
    // Create the menu task.
    if(xTaskCreate(ActuatorTask, (const portCHAR *)"actuator",
                   ACTUATORTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_ACTUATOR_TASK, NULL) != pdTRUE)
    {
        return(0);
    }

    // Success.
    return(1);
}



