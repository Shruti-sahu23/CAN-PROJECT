#ifndef __PROJECT_FUNCTIONS_H__
#define __PROJECT_FUNCTIONS_H__

#include "types.h"
#include "CAN.h"

/* Function Prototypes */
u8   Process_Window_Control(u8 action, u8 current_level, u8 *fault_flag);
void Process_Reverse_Mode(u8 *fault_flag);

#endif /* __PROJECT_FUNCTIONS_H__ */
