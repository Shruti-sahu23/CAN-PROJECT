#ifndef __CAN_DEFINES_H__
#define __CAN_DEFINES_H__

/* CAN Message Identifiers */
#define CAN_ID_WINDOW_CTRL      0x101  /* Main Node -> Window Node */
#define CAN_ID_WINDOW_STATUS    0x201  /* Window Node -> Main Node */

/* Window Control Commands */
#define WINDOW_CMD_IDLE         0x00
#define WINDOW_CMD_UP           0x01
#define WINDOW_CMD_DOWN         0x02

/* Status Flags */
#define WINDOW_STATUS_OPEN      0x00
#define WINDOW_STATUS_CLOSED    0x64  /* 100% */

#endif /* __CAN_DEFINES_H__ */
