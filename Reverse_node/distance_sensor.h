#ifndef __DISTANCE_SENSOR_H__
#define __DISTANCE_SENSOR_H__

#include "types.h"
#include "adc.h"
#include "CAN_Defines.h"

/* Sensor Constants */
#define DIST_SAMPLE_COUNT       20
#define DIST_DEADBAND_CM        2.0f

/* Function Prototypes */
void Distance_Sensor_Init(void);
u8   Distance_Calculate_cm(void);
u8   Distance_Determine_Zone(u8 distance_cm);

#endif /* __DISTANCE_SENSOR_H__ */
