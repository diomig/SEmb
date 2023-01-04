#ifndef INC_LCD_H_
#define INC_LCD_H_

/*********** Define the LCD PINS below ****************/

/*#define RS_Pin GPIO_PIN_1
#define RS_GPIO_Port GPIOA*/
#define RS_Pin GPIO_PIN_3
#define RS_GPIO_Port GPIO_PORTA_BASE
#define EN_Pin GPIO_PIN_2
#define EN_PORT GPIO_PORTA_BASE
#define D4_Pin GPIO_PIN_4
#define D4_PORT GPIO_PORTC_BASE
#define D5_Pin GPIO_PIN_5
#define D5_PORT GPIO_PORTC_BASE
#define D6_Pin GPIO_PIN_6
#define D6_PORT GPIO_PORTC_BASE
#define D7_Pin GPIO_PIN_7
#define D7_PORT GPIO_PORTC_BASE



void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_cmd (char cmd);

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);

void delay_ms(uint16_t u16milisecond);

#endif /* INC_LCD_H_ */
