/*
 * temp_task.h
 *
 *  Created on: 07/01/2023
 *      Author: diomi
 */

#ifndef TEMP_TASK_H_
#define TEMP_TASK_H_

extern uint32_t TempTaskInit(void);


int checkLimits();
uint8_t temp_Duty_Cycle(float temperature);

#endif /* TEMP_TASK_H_ */
