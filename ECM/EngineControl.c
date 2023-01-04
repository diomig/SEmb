//*****************************************************************************
//Engine Control Module
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "lcd.h"

void keypadHandler();
void DebounceHandler();

void pinConfiguration(void);
void timerConfiguration(void);
int date_setup(void);
int time_setup(void);
int8_t getColumn(uint8_t rowbits);
int8_t getRow(uint8_t colbits);
int8_t bitsToIndex(int8_t bits);
void menu(void);
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif




//*****************************************************************************
//*****************************************************************************
const uint8_t keypad[4][4] = {    {'1','2','3','F'},
                                  {'4','5','6','E'},
                                  {'7','8','9','D'},
                                  {'A','0','B','C'}};

volatile uint8_t key = 0;




int main(void)
    {
    timerConfiguration();
    pinConfiguration();

    lcd_init();
    lcd_clear();
    lcd_put_cur(0);


    int date = date_setup();
    int time = time_setup();
    lcd_clear();

    // Loop forever.
    while(1)
    {
        menu();

    }
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Interrupt Handlers
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void keypadHandler(){
    int8_t column, row;

    int status = GPIOIntStatus(GPIO_PORTD_BASE,true);
    GPIOIntClear(GPIO_PORTD_BASE,status);

    GPIOIntDisable(GPIO_PORTD_BASE, 0x0F);   //temporarily disable interrupts for debounce
    TimerLoadSet(TIMER0_BASE, TIMER_A, 10000);
    //TimerEnable(TIMER0_BASE, TIMER_A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    column = GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
    if (column != 0)
        row = getRow(column);
    column = bitsToIndex(column);
    row = bitsToIndex(row);
    if (row>=0 && row<=3 && column>=0 && column<=3)
        key = keypad[row][column];

    GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);
}

void DebounceHandler(){
    //TimerDisable(TIMER0_BASE, TIMER_A);
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);   //reactivate keypad interrupts


}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Function Declarations
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void pinConfiguration(void) {
    //
        // Enable the GPIO port that is used for the on-board LED.
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);


        GPIOIntTypeSet(GPIO_PORTD_BASE, 0x0F, GPIO_RISING_EDGE);
        GPIOIntRegister(GPIO_PORTD_BASE, keypadHandler);
        GPIOIntEnable(GPIO_PORTD_BASE, 0x0F);

        //
        // Check if the peripheral access is enabled.
        //
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        {
        }
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        {
        }

        GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3);
        GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, 0xF0);
        GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, 0x0F);
        GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x0F);

        GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, 0x0F);

}


void timerConfiguration(void) {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xFFFF);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, DebounceHandler);
    TimerEnable(TIMER0_BASE, TIMER_A);

}

int date_setup(void){
    int len = 0;
    int date;
    int position[8] = {6,7,9,10,12,13,14,15};
    int ord_mag[8] = {100000,10000,10000000,100000000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Date: MM/DD/YYYY");
    for(len = 0; len < 8; len++) {
        lcd_put_cur(position[len]);
        while(!key);
        if(key == 'C'){
            len -= 2;
            if(len<0){len=-1;}
        }
        else if(key < 'A'){
            lcd_send_data(key);
            date = (key - '0')*ord_mag[len];
        }
        key=0;
    }
    return date;
}
int time_setup(void){
    int len = 0;
    int time;
    int position[6] = {6,7,9,10,12,13};
    int ord_mag[6] = {100000,10000,1000,100,10,1};
    lcd_clear();
    lcd_put_cur(0);
    lcd_send_string("Time: hh:mm:ss");
    for(len = 0; len < 6; len++) {
        lcd_put_cur(position[len]);
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

int8_t getColumn(uint8_t rowbits) {
    GPIOPinWrite(GPIO_PORTE_BASE, 0x0F, rowbits);

    return GPIOPinRead(GPIO_PORTD_BASE, 0x0F);
}

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

int8_t bitsToIndex(int8_t bits){
    switch(bits) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    default:
        return -1;
    }
}

void menu(void){
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



