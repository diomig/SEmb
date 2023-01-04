/*************************************************************************
 * LCD
 *
 ************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"


#include <lcd.h>

/****************************************************************************************************************************************************************/

void send_to_lcd (char data, int rs)
{
    GPIOPinWrite(RS_GPIO_Port, RS_Pin, rs*RS_Pin);  // rs = 1 for data, rs=0 for command

    /* write the data to the respective pin */
    GPIOPinWrite(D7_PORT, 0xF0, (data<<4));

    /* Toggle EN PIN to send the data
     * if the HCLK > 100 MHz, use the  20 us delay
     * if the LCD still doesn't work, increase the delay to 50, 80 or 100..
     */
    GPIOPinWrite(EN_PORT, EN_Pin, EN_Pin);
    delay_ms (20);
    GPIOPinWrite(EN_PORT, EN_Pin, 0);
    delay_ms (20);
}

void lcd_send_cmd (char cmd)
{
    char datatosend;

    /* send upper nibble first */
    datatosend = ((cmd>>4)&0x0f);
    send_to_lcd(datatosend,0);  // RS must be 0 while sending command

    /* send Lower Nibble */
    datatosend = ((cmd)&0x0f);
    send_to_lcd(datatosend, 0);
}


void lcd_cmd (char cmd)
{
    send_to_lcd(cmd,0);  // RS must be 0 while sending command
}

void lcd_send_data (char data)
{
    char datatosend;

    /* send higher nibble */
    datatosend = ((data>>4)&0x0f);
    send_to_lcd(datatosend, 1);  // rs =1 for sending data

    /* send Lower nibble */
    datatosend = ((data)&0x0f);
    send_to_lcd(datatosend, 1);
}

void lcd_clear (void)
{
    lcd_send_cmd(0x01);
    delay_ms(2);
}

void lcd_put_cur(int col)
{
    col |= 0x80;
    lcd_send_cmd (col);
}


void lcd_init (void)
{
    send_to_lcd(0x00, 0);
    delay_ms(40);
    lcd_cmd(0x03);
    delay_ms(10);
    lcd_cmd(0x03);
    delay_ms(10);
    lcd_cmd(0x03);
    /////////////////////////////////////////////////////
    lcd_cmd(0x02);
    lcd_cmd(0x02);//Function set 1, 0-4bits
    lcd_cmd(0x00);// nº linhas  font 5x8 Nº de linhas 1

    lcd_cmd(0x00);// display on/off
    lcd_cmd(0x0F);// 1, Display-on, Cursor - 1, Blink -0


    lcd_cmd(0x00);// entry mode set
    lcd_cmd(0x06);// increment the address by 1, shift the cursor to right
}

void lcd_send_string (char *str)
{
    while (*str) lcd_send_data (*str++);
}


void delay_ms(uint16_t u16milisecond)
{
    SysCtlDelay(SysCtlClockGet() / 3000 * u16milisecond);//1ms
}
