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
#include "switch_task.h"
#include "led_task.h"
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

//*****************************************************************************
//
// This task reads the buttons' state and passes this information to LEDTask.
//
//*****************************************************************************
static void
MenuTask(void *pvParameters)
{
    uint8_t key;
    float temp;
    char tempstr[20];
    int date = date_setup();
    int time = time_setup();
    lcd_clear();


    I2CInit();
    I2CWriteConf(0x60);         //em vez de 0x40 escrevi 0x60 para termos o resultado da medicao em 12 bits (se quiseres podemos dps alterar mas isso influencia os shifts da leitura

    // Loop forever.
    while(1)
    {
        /*
        if (xQueueReceive(keypadQueue, &key, portMAX_DELAY) == pdPASS){
            menu(key);
        }
        */



        temp = ReadTemp();
        //lcd_clear();
        lcd_put_cur(0);

        sprintf(tempstr,"T = %.2f %cC", temp, 0xDF);
        lcd_send_string(tempstr);
        vTaskDelay(500);

    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
MenuTaskInit(void)
{
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







int date_setup(void){
    uint8_t key;
    int len = 0;
    int date;
    int position[8] = {6,7,9,10,12,13,14,15};
    int ord_mag[8] = {100000,10000,10000000,100000000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Date: MM/DD/YYYY");
    for(len = 0; len < 8; len++) {
        lcd_put_cur(position[len]);

        xQueueReceive(keypadQueue, &key, portMAX_DELAY);

        if(key == 'C'){
            len -= 2;
            if(len<0){len=-1;}
        }
        else if(key < 'A'){
            lcd_send_data(key);
            date = (key - '0')*ord_mag[len];
        }
        else {
            lcd_send_data('n');
            continue;
        }
        key=0;
    }
    return date;
}
int time_setup(void){
    uint8_t key;
    int len = 0;
    int time;
    int position[6] = {6,7,9,10,12,13};
    int ord_mag[6] = {100000,10000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Time: hh:mm:ss");
    for(len = 0; len < 6; len++) {
        lcd_put_cur(position[len]);
        xQueueReceive(keypadQueue, &key, portMAX_DELAY);

        while(!key);
        if(key == 'C'){
            len -= 2;
            if(len<0){len=-1;}
        }
        else if(key < 'A'){
            lcd_send_data(key);
            time = (key - '0') * ord_mag[len];
        }
        key=0;
    }
    return time;
}



void menu(uint8_t key){
    if (key==0) {return;}

    switch(key){
    case 'A':
        lcd_put_cur(0);
        lcd_send_string("Action A");
        break;
    case 'B':
        lcd_put_cur(0);
        lcd_send_string("Action B");
        break;
    case 'C':
        lcd_clear();
        break;
    case 'D':
        lcd_put_cur(0);
        lcd_send_string("Action D");
        break;
    case 'E':
        lcd_put_cur(0);
        lcd_send_string("Action E");
        break;
    case 'F':
        lcd_put_cur(0);
        lcd_send_string("Action F");
        break;
    default:
        lcd_send_data(key);
    }
    key = 0;
}




float ReadTemp (void){
    float temp;

    temp = I2CRead(REG_TEMP_ADDR);

    temp = temp * 0.0625;

    return temp;
}

