#ifndef __CAN_DEFINES_H__
#define __CAN_DEFINES_H__
 
/* Standard CAN Message IDs for Distributed System */
#define CAN_ID_WINDOW_CTRL      0x101  /* Main Node -> Window Node */
#define CAN_ID_REVERSE_ENABLE   0x102  /* Main Node -> Reverse Node */
#define CAN_ID_DISTANCE_DATA    0x103  /* Reverse Node -> Main Node */
#define CAN_ID_WINDOW_STATUS    0x201  /* Window Node -> Main Node */

/* Window Control Commands */
#define WINDOW_CMD_IDLE         0x00
#define WINDOW_CMD_UP           0x01
#define WINDOW_CMD_DOWN         0x02

/* Reverse Mode Commands */
#define REVERSE_DISABLE         0x00
#define REVERSE_ENABLE          0x01

/* Proximity Zones */
#define ZONE_SAFE               0x01
#define ZONE_WARNING            0x02
#define ZONE_DANGER             0x03
#define ZONE_CRITICAL           0x04

#endif /* __CAN_DEFINES_H__ */
