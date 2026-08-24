#ifndef __CAN_DEFINES_H__
#define __CAN_DEFINES_H__
 
/* CAN Message Identifiers */
#define CAN_ID_REVERSE_ENABLE   0x102  /* Main Node -> Reverse Node */
#define CAN_ID_DISTANCE_DATA    0x103  /* Reverse Node -> Main Node */
 
/* Reverse Commands */
#define REVERSE_DISABLE         0x00
#define REVERSE_ENABLE          0x01

/* Proximity Zone Thresholds */
#define ZONE_SAFE               0x01
#define ZONE_WARNING            0x02
#define ZONE_DANGER             0x03
#define ZONE_CRITICAL           0x04

#endif /* __CAN_DEFINES_H__ */
