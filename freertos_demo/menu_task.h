/*
 * menu_task.h
 *
 *  Created on: 04/01/2023
 *      Author: diomi
 */

#ifndef MENU_TASK_H_
#define MENU_TASK_H_

uint32_t MenuTaskInit(void);


int date_setup(void);
int time_setup(void);
void menu(uint8_t key);

float ReadTemp (void);

#endif /* MENU_TASK_H_ */
