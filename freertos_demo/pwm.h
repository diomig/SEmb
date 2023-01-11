/*
 * pwc.h
 *
 *  Created on: 10/01/2023
 *      Author: diomi
 */

#ifndef PWM_H_
#define PWM_H_

void PWMInit (void);
void setSpeaker(int freq);
void stopSeaker(void);
void setFan(unsigned int duty_cycle);
void stopFan(void);

#endif /* PWM_H_ */
