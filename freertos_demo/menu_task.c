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

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define MENUTASKSTACKSIZE        1000         // Stack size in words



extern xQueueHandle keypadQueue;
extern float motorTemp;
extern uint8_t MinTEMP;
extern uint8_t MaxTEMP;

extern xSemaphoreHandle LCDMutex;
extern xSemaphoreHandle tempMutex;
//*****************************************************************************
//
// This task reads the buttons' state and passes this information to LEDTask.
//
//*****************************************************************************
static void
MenuTask(void *pvParameters)
{
    uint8_t key;
    int ticks;
    float temp;
    char tempstr[20];
    int time, date;


    int resetDate = date_setup();
    int resetTime = time_setup();

    lcd_clear();


    I2CInit();
    I2CWriteConf(0x60);         //em vez de 0x40 escrevi 0x60 para termos o resultado da medicao em 12 bits (se quiseres podemos dps alterar mas isso influencia os shifts da leitura

    // Loop forever.
    while(1)
    {
        showTime(getTime(resetTime));
        if (xQueueReceive(keypadQueue, &key, 0) == pdPASS){
            menu(key, &resetTime);
        }



    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t MenuTaskInit(void) {
    // Create the menu task.
    if(xTaskCreate(MenuTask, (const portCHAR *)"menu",
                   MENUTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_MENU_TASK, NULL) != pdTRUE)
    {
        return(0);
    }

    // Success.
    return(1);
}










void menu(uint8_t key, int* resettime){
   /* float temp;
    char tempstr[20];


    xSemaphoreTake(tempMutex, portMAX_DELAY);
    temp = motorTemp;
    xSemaphoreGive(tempMutex);
*/
    if (key==0) {return;}

    switch(key){
    case 'A':
        showTemperature();
        break;
    case 'B':
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_put_cur(0);
        lcd_send_string("Action B");
        xSemaphoreGive(LCDMutex);
        break;
    case 'C':
        MinTEMP = setMinTemp();
        break;
    case 'D':
        MaxTEMP = setMaxTemp();
        break;
    case 'E':
        *resettime = time_setup();
        break;
    case 'F':
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_clear();
        xSemaphoreGive(LCDMutex);
        break;
    default:
        //xSemaphoreTake(LCDMutex, portMAX_DELAY);
        //lcd_send_data(key);
        showTime(getTime(*resettime));
        //xSemaphoreGive(LCDMutex);

    }
    key = 0;
}


int date_setup(void){
    uint8_t key, previouskey;
    int i = 0;
    int date;
    int position[8] = {6,7,9,10,12,13,14,15};
    int ord_mag[8] = {100000,10000,10000000,100000000,1000,100,10,1};
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Date: MM/DD/YYYY");
    while(i < 8) {
        lcd_put_cur(position[i]);

        xQueueReceive(keypadQueue, &key, portMAX_DELAY);

        if(key == 'C'){
            i --;
            if(i<0){i=0;}
            date -= (previouskey - '0') * ord_mag[i];   //cancel last assignment
            continue;
        }
        else if(key < 'A'){
            lcd_send_data(key);
            date += (key - '0')*ord_mag[i];
        }
        else {
            continue;
        }
        previouskey = key;
        i++;
    }
    lcd_clear();
    xSemaphoreGive(LCDMutex);

    return date;
}
int time_setup(void){
    uint8_t key, previouskey;
    int i = 0;
    int time=0;
    int position[6] = {6,7,9,10,12,13};
    int ord_mag[6] = {36000,3600,600,60,10,1};
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Time: hh:mm:ss");
    while(i < 6) {
        lcd_put_cur(position[i]);
        xQueueReceive(keypadQueue, &key, portMAX_DELAY);

        if(key == 'C'){
            i--;
            if(i<0){i=0;}
            time -= (previouskey - '0') * ord_mag[i];    //cancel last assignment
            continue;
        }
        else if(keyIsNumber(key)){
            lcd_send_data(key);
            time += (key - '0') * ord_mag[i];
        }
        previouskey = key;

        i++;
    }
    lcd_clear();
    xSemaphoreGive(LCDMutex);
    return time - xTaskGetTickCount()/1000;
}




int getTime(int resetTime) {
    return (resetTime + xTaskGetTickCount()/1000);
}

float ReadTemp (void){
    float temp;

    temp = I2CRead(REG_TEMP_ADDR);

    temp = temp * 0.0625;

    return temp;
}


void showTemperature(){
    uint8_t key;
    float temp;
    char tempstr[20];
    bool loop = true;

    lcd_clear();

    while(loop){
        xSemaphoreTake(tempMutex, portMAX_DELAY);
        temp = motorTemp;
        xSemaphoreGive(tempMutex);

        sprintf(tempstr,"T = %.2f %cC", temp, 0xDF);
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_put_cur(0);
        lcd_send_string(tempstr);

        //vTaskDelay(1000);

        xQueueReceive(keypadQueue, &key, 500);
        if (keyIsValid(key)){
            loop = false;
            lcd_clear();
        }
        xSemaphoreGive(LCDMutex);
    }
}



uint8_t setMaxTemp() {
    uint8_t maxtemp = MaxTEMP;
    uint8_t key, i = 0;
    char msg[21];
    const int magnitude[3] = {100, 10, 1};
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_clear();
    lcd_put_cur(0);
    sprintf(msg, "Tmax = %3d %cC", maxtemp, 0xDF);
    lcd_send_string(msg);

    //xQueueReceive(keypadQueue, &key, portMAX_DELAY);
    maxtemp = 0;
    while(i < 3){
        lcd_put_cur(i+7);
        xQueueReceive(keypadQueue, &key, portMAX_DELAY);
        if(keyIsNumber(key)){
            lcd_send_data(key);
            maxtemp += (key-'0')*magnitude[i];
            i++;
        }
    }
    lcd_clear();

    xSemaphoreGive(LCDMutex);

    return maxtemp;
}


uint8_t setMinTemp() {
    uint8_t mintemp = MinTEMP;
    uint8_t key, i = 0;
    char msg[21];
    const int magnitude[3] = {100, 10, 1};
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_clear();
    lcd_put_cur(0);
    sprintf(msg, "Tmin = %3d %cC", mintemp, 0xDF);
    lcd_send_string(msg);

    //xQueueReceive(keypadQueue, &key, portMAX_DELAY);
    mintemp = 0;
    while(i < 3){
        lcd_put_cur(i+7);
        xQueueReceive(keypadQueue, &key, portMAX_DELAY);
        if(keyIsNumber(key)){
            lcd_send_data(key);
            mintemp += (key-'0')*magnitude[i];
            i++;
        }
    }
    lcd_clear();

    xSemaphoreGive(LCDMutex);

    return mintemp;
}


void showTime(int time) {
    char timestr[20];

    sprintf(timestr, "%02d:%02d:%02d", time/3600%24, time/60%60, time%60);
    //sprintf(timestr, "%d", time);
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_put_cur(0);
    lcd_send_string(timestr);
    xSemaphoreGive(LCDMutex);

    vTaskDelay(500);
}


