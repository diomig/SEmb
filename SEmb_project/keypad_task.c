/*
 * keypad_task.c
 *
 *  Created on: 03/01/2023
 *      Author: diomi
 */



#include "keypad_task.h"

#define KEY_ITEM_SIZE sizeof(uint8_t)
#define KEY_QUEUE_SIZE 1


xSemaphoreHandle keypadSemaphore;
//*****************************************************************************
//*****************************************************************************

//matriz do teclado
const uint8_t keypad[4][4] = {    {'1','2','3','F'},
                                  {'4','5','6','E'},
                                  {'7','8','9','D'},
                                  {'A','0','B','C'}};


//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define KEYPADTASKSTACKSIZE        1000         // Stack size in words

xQueueHandle  keypadQueue;

//*****************************************************************************
//
// This task reads the keypad and passes key value to the control unit
//
//*****************************************************************************

int lastState = 0;
int currentState = 0;

void KeypadInterruptHandler(void) {
    static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    int status = GPIOIntStatus(GPIO_PORTD_BASE,true);
    GPIOIntClear(GPIO_PORTD_BASE,status);

    //GPIOIntDisable(GPIO_PORTD_BASE, 0x0F);   //temporarily disable interrupts for debounce
    //TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFF);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    delay_ms(20);
    currentState = (GPIOPinRead(GPIO_PORTD_BASE, 0x0F)!=0);
    /* 'Give' the semaphore to unblock the task. */
    if (lastState == 0 && currentState==1){
        xSemaphoreGiveFromISR( keypadSemaphore, &xHigherPriorityTaskWoken );
    }
    lastState = currentState;

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
        column = GPIOPinRead(GPIO_PORTD_BASE, 0x0F);    //mascara correspondente 'a coluna
        if (column != 0)
            row = getRow(column);                       //mascara correspondente 'a linha
        column = bitsToIndex(column);                   //coluna da tecla premida
        row = bitsToIndex(row);                         //linha da tecla premida
        if (row>=0 && row<=3 && column>=0 && column<=3)
            key = keypad[row][column];                  //tecla premida

        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);      //voltar a alimentar as linhas do teclado

        //GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts

        // Pass the value of the key pressed to the menu task

        if (keyIsValid(key)) {
            vTaskDelay(40);
            currentState = 0;
            lastState = 0;
            xQueueSend(keypadQueue, &key, 0);       //envia a tecla premida para a tarefa do menu
        }

        //GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts
    }
}

//verifica se key corresponde a uma tecla valida
int keyIsValid(uint8_t key){
    if (key < '0' || key > 'F')
        return 0;
    return 1;
}

//verifica se a tecla premida e' um numero
int  keyIsNumber(uint8_t key) {
    return (key >= '0' && key <= '9');
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
    keypadQueue = xQueueCreate(KEY_QUEUE_SIZE, KEY_ITEM_SIZE);  //cria a queue do teclado

    vSemaphoreCreateBinary(keypadSemaphore);    //cria semaforo para a keypad handler task
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



//obtem bits do porto das colunas
int8_t getColumn(uint8_t rowbits) {
    GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, rowbits);

    return GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
}

//obtem bits do porto das linhas
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

//converte os bits do porto das linhas/colunas no indice correspondente
int8_t bitsToIndex(int8_t bits){
    switch(bits) {
    case 1:         //0001 -> 0
        return 0;
    case 2:         //0010 -> 1
        return 1;
    case 4:         //0100 -> 2
        return 2;
    case 8:         //1000 -> 3
        return 3;
    default:
        return -1;  //invalido
    }
}
