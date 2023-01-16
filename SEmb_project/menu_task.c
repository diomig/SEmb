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
#include "menu_task.h"
#include "lcd.h"
#include "i2c.h"
#include "actuator_task.h"



#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define MENUTASKSTACKSIZE   1000         // Stack size in words
#define STARTBUTTON         '0'

extern xQueueHandle msgQueue;
extern xQueueHandle keypadQueue;
extern float motorTemp;
extern uint8_t duty_cycle;
extern uint8_t MinTEMP;
extern uint8_t MaxTEMP;

extern xSemaphoreHandle LCDMutex;
extern xSemaphoreHandle tempMutex;
extern xSemaphoreHandle fanMutex;


int resetDate[8];
int resetTime;


//*****************************************************************************
//
// This task reads the buttons' state and passes this information to LEDTask.
//
//*****************************************************************************
static void MenuTask(void *pvParameters) {

    extern xQueueHandle msgQueue;
    MESSAGE message;
    uint8_t key;
    //int ticks;
    //float temp;
    //char tempstr[20];
    //int time, date;



    date_setup(resetDate);
    resetTime = time_setup();


    I2CInit();
    I2CWriteConf(0x60);


    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Press 0 to start");
    xSemaphoreGive(LCDMutex);

    do{
        xQueueReceive(keypadQueue, &key, portMAX_DELAY);
    }
    while(key != STARTBUTTON);
    message.msg_id = ID_START;
    xQueueSend(msgQueue, &message, 0);

    // Loop forever.
    while(1)
    {
        showTime(getTime(resetTime), resetDate);
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
    MESSAGE message;

    if (key==0) {return;}

    switch(key){
    case 'A':
        showTemperature();
        break;
    case 'B':
        showFanSpeed();
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
        date_setup(resetDate);
        break;
    default:
        message.msg_id = ID_FAN_DUTY_CYCLE;
        message.msg_value = (key-'0')*10;
        xQueueSend(msgQueue, &message, 0);
    }
    key = 0;
}

int dateIsValid(uint8_t digit, int* date, int position){
    int max[8] = {1, 9, 3, 9, 9, 9, 9, 9};
    if(digit > max[position]){
        return 0;
    }
    if (position == 1){
        if (digit + date[0]*10 <1 || digit + date[0]*10 > 12){
            return 0;
        }
    }
    if (position == 3){
        if (digit + date[2]*10 > 31){
            return 0;
        }
    }

    return 1;
}


void date_setup(int* Date){
    uint8_t key  ;
    int i = 0;
    int position[8] = {6,7,9,10,12,13,14,15};
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
            continue;
        }
        else if(keyIsNumber(key)){
            if (!dateIsValid(key-'0', Date, i)){continue;}
            lcd_send_data(key);
            Date[i] = key-'0';
        }
        else {
            continue;
        }

        i++;
    }
    lcd_clear();
    xSemaphoreGive(LCDMutex);

}


int timeIsValid(uint8_t digit, int time, int position){
    int max_seconds = 86399;    //24*3600-1
    int max[6] = {2, 9, 5, 9, 5, 9};
    int ord_mag[6] = {36000,3600,600,60,10,1};
    if(digit > max[position]){
        return 0;
    }
    if (time + digit*ord_mag[position] > max_seconds){
        return 0;
    }
    return 1;
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
            if (!timeIsValid(key-'0', time, i)){continue;}
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
    uint16_t maxtemp = MaxTEMP;
    uint8_t key, previouskey = 0, i = 0;
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
            previouskey = key;
            i++;
        }
        else if (key == 'C'){
            i--;
            maxtemp -= (previouskey-'0')*magnitude[i];
        }
    }

    lcd_clear();

    xSemaphoreGive(LCDMutex);

    if (maxtemp <= MinTEMP){
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_put_cur(0);
        lcd_send_string("Tmax <= Tmin!!!");
        xSemaphoreGive(LCDMutex);
        vTaskDelay(1000);
        maxtemp = setMaxTemp();
    }

    return maxtemp;
}


uint8_t setMinTemp() {
    uint8_t mintemp = MinTEMP;
    uint8_t key, previouskey = 0, i = 0;
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
            previouskey = key;
            i++;
        }
        else if(key == 'C'){
            i--;
            mintemp -= (previouskey-'0')*magnitude[i];
        }
    }
    lcd_clear();

    xSemaphoreGive(LCDMutex);


    if (mintemp >= MaxTEMP){
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_put_cur(0);
        lcd_send_string("Tmin >= Tmax!!!");
        xSemaphoreGive(LCDMutex);
        vTaskDelay(1000);
        mintemp = setMinTemp();
    }



    return mintemp;
}


void showTime(int time, int* date) {
    char timestr[20];
    int day = date[2]*10 + date[3];
    int month = date[0]*10 + date[1];
    char* month_name[12] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
    sprintf(timestr, "%02d:%02d:%02d      %02d %s", time/3600%24, time/60%60, time%60, day, month_name[month-1]);
    //sprintf(timestr, "%d", time);
    xSemaphoreTake(LCDMutex, portMAX_DELAY);
    lcd_put_cur(0);
    lcd_send_string(timestr);
    xSemaphoreGive(LCDMutex);

    vTaskDelay(500);
}


void showFanSpeed(){
    uint8_t key, duty;
    bool loop = true;
    char str[21];

    lcd_clear();

    while(loop){
        xSemaphoreTake(fanMutex, portMAX_DELAY);
        duty = duty_cycle;
        xSemaphoreGive(fanMutex);

        sprintf(str,"Duty Cycle = %3d %%", duty);
        xSemaphoreTake(LCDMutex, portMAX_DELAY);
        lcd_put_cur(0);
        lcd_send_string(str);

        //vTaskDelay(1000);

        xQueueReceive(keypadQueue, &key, 500);
        if (keyIsValid(key)){
            loop = false;
            lcd_clear();
        }
        xSemaphoreGive(LCDMutex);
    }
}
