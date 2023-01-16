/*
 * menu_task.h
 *
 *  Created on: 04/01/2023
 *      Author: diomi
 */

#ifndef MENU_TASK_H_
#define MENU_TASK_H_

uint32_t MenuTaskInit(void);


void date_setup(int* Date);
int time_setup(void);
int timeIsValid(uint8_t key, int time, int position);
void menu(uint8_t key, int* resettime);
float ReadTemp (void);
int getTime();
void showTime(int resetTime, int* date);
void showTemperature();
void showFanSpeed();
uint8_t setMaxTemp(void);
uint8_t setMinTemp(void);

#endif /* MENU_TASK_H_ */
