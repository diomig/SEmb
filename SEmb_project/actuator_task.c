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
    bool FAN_WORKING = false;       //indica se a ventoinha esta' parada

    msgQueue = xQueueCreate(MSG_QUEUE_SIZE, MSG_ITEM_SIZE);  //cria queue de comunicacao entre a
                                                            //centralina e os atuadores

    do{
        xQueueReceive(msgQueue, &message, portMAX_DELAY);
    }
    while(message.msg_id != ID_START);  //espera comando START (tecla '0')


    // Loop forever.
    while(1)
    {
        xQueueReceive(msgQueue, &message, portMAX_DELAY);   //espera por um comando
        if(message.msg_id == ID_FAN_DUTY_CYCLE) {   //comando para ligar a ventoinha com determinado duty cycle
            if(!FAN_WORKING){   //caso a ventoinha esteja inicialmente parada, arranca a 50% durante 1s
                setFan(50);
                vTaskDelay(1000);
                FAN_WORKING = true;
            }
            setFan(message.msg_value);  //define o duty cycle da ventoinha
        }
        else if (message.msg_id == ID_FAN_STOP){    //comando para desligar a ventoinha
            setFan(0);
            FAN_WORKING = false;
        }
        else if(message.msg_id == ID_ALARM_SET){    //comando para ligar o alarme
            setSpeaker();
        }
        else if(message.msg_id == ID_ALARM_MUTE){   //comando para desligar o alarme
            stopSeaker();
        }
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



