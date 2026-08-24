#ifndef __DASHBOARD_H__
#define __DASHBOARD_H__

#include "types.h"
#include "LCD.h"

/* Temperature Warning Thresholds */
#define TEMP_THRESHOLD_WARMING   45.0f
#define TEMP_THRESHOLD_HIGH      60.0f
#define TEMP_THRESHOLD_CRITICAL  75.0f
#define TEMP_HYSTERESIS          2.0f

/* Node Identification Codes for Error Detection */
#define NODE_TYPE_WINDOW         0x01
#define NODE_TYPE_REVERSE        0x02
#define NODE_TYPE_TEMP_SENSOR    0x03

/* Function Prototypes */
void display_splash_screen(void);
void display_dashboard(f32 temp, u8 window_level, u8 reverse_state, u8 ecu_fault_flag);
void display_window_screen(u8 action, u8 level);
void display_reverse_radar(u8 distance_cm, u8 zone);
void display_node_error_screen(u8 node_type);

#endif /* __DASHBOARD_H__ */
