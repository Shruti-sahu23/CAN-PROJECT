/**
 * @file CAN.h
 * @brief Reverse Node CAN Driver Header File with VIC RX Interrupt ISR
 * @author Shruti Sahu
 */ 

#ifndef __CAN_H__ /* Prevent multiple inclusions */
#define __CAN_H__ /* Define header guard token */

#include <lpc21xx.h>     /* Include LPC2129 register definitions */
#include "types.h"       /* Include standard data types */
#include "CAN_Defines.h" /* Include CAN Protocol Identifier Constants */

#define VIC_CHANNEL_CAN1    26       /* VIC Interrupt Channel 26 for CAN1 Controller */
#define VIC_ENABLE_BIT      (1 << 5) /* Bit 5 of VICVectCntl register enables vector slot */

typedef struct {
    u32 id;       /**< 11-bit standard CAN identifier */
    u8  dlc;      /**< Data Length Code (0 to 8 bytes) */
    u8  rtr;      /**< Remote Transmission Request flag */
    u8  data[8];  /**< Payload data array */
} CAN_MSG;        /* CAN message frame structure typedef */

extern volatile CAN_MSG g_can_rx_msg;  /* Global message structure updated by CAN RX ISR */
extern volatile u8      g_can_rx_flag; /* Volatile flag set to 1 when ISR receives frame */

void CAN1_Init(void);         /* Function prototype to initialize CAN1 & VIC interrupt */
void CAN1_Rx_ISR(void) __irq; /* Hardware ISR handler function for CAN1 Receive Interrupt */
u8   CAN1_Tx(CAN_MSG *msg);   /* Transmits a CAN frame using hardware Tx Buffer 1 */
u8   CAN1_Rx(CAN_MSG *msg);   /* Non-blocking read from ISR received message buffer */

#endif /* __CAN_H__ End of header guard */
