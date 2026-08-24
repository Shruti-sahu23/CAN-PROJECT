#ifndef __WINDOW_CONTROL_H__
#define __WINDOW_CONTROL_H__

#include <lpc21xx.h>
#include "types.h"
#include "CAN.h"

/* Window Position Indicator / Actuator Pins on P0.0 - P0.7 */
#define WINDOW_LED_MASK     (0xFFU)      /* P0.0 - P0.7 */
#define WINDOW_MAX_LEVEL    8            /* 8 Discrete Physical Levels */

/* Function Prototypes */
void Window_Control_Init(void);
u8   Window_Step_Up(void);
u8   Window_Step_Down(void);
void Window_Update_LEDs(u8 level);
void Window_Execute_Command(u8 cmd);

#endif /* __WINDOW_CONTROL_H__ */
