

#ifndef MAIN_H_
#define MAIN_H_


#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_ints.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/debug.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "driverlib/rom_map.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "keypad_task.h"
#include "menu_task.h"
#include "temp_task.h"
#include "actuator_task.h"
#include "pwm.h"
#include "i2c.h"
#include "lcd.h"


#endif /* MAIN_H_ */
