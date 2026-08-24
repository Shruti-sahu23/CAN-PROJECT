/**
 * @file CAN.h
 * @brief LPC2129 Hardware CAN Controller Driver Header File with VIC RX Interrupt ISR
 * @author Shruti Sahu
 */ 

#ifndef __CAN_H__ /* Prevent multiple inclusions of header file */
#define __CAN_H__ /* Define header guard token */

#include <lpc21xx.h>     /* Include LPC2129 register definitions */
#include "types.h"       /* Include standard fixed-width data types */
#include "CAN_defines.h" /* Include CAN Protocol Identifier & Command Constants */

/*====================================================================
 *                VIC INTERRUPT CHANNEL CONSTANTS
 *====================================================================*/
#define VIC_CHANNEL_CAN1    26       /* VIC Interrupt Channel 26 for CAN1 Controller */
#define VIC_ENABLE_BIT      (1 << 5) /* Bit 5 of VICVectCntl register enables vector slot */

/*====================================================================
 *                    CAN MESSAGE FRAME STRUCTURE
 *====================================================================*/
typedef struct {
    u32 id;       /**< 11-bit standard or 29-bit extended CAN identifier */
    u8  dlc;      /**< Data Length Code specifying number of bytes (0 to 8) */
    u8  rtr;      /**< Remote Transmission Request flag (0 = Data Frame, 1 = RTR) */
    u8  data[8];  /**< Array holding up to 8 data payload bytes */
} CAN_MSG;        /* Typedef structure name for CAN message frames */

/*====================================================================
 *           VOLATILE INTERRUPT RECEIVED MESSAGE GLOBALS
 *====================================================================*/
extern volatile CAN_MSG g_can_rx_msg;  /* Global message structure updated by CAN RX ISR */
extern volatile u8      g_can_rx_flag; /* Volatile flag set to 1 when ISR receives frame */

/*====================================================================
 *                  FUNCTION PROTOTYPES
 *====================================================================*/
void CAN1_Init(void);             /* Initializes CAN1 hardware controller & VIC interrupt */
void CAN1_Rx_ISR(void) __irq;     /* Hardware ISR handler function for CAN1 Receive Interrupt */
u8   CAN1_Tx(CAN_MSG *msg);       /* Transmits a CAN frame using hardware Tx Buffer 1 */
u8   CAN1_Rx(CAN_MSG *msg);       /* Non-blocking read from ISR received message buffer */

#endif /* __CAN_H__ End of header guard */
