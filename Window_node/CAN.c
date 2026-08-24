/**
 * @file CAN.c
 * @brief Window Node CAN Controller Driver Implementation with VIC Interrupt ISR
 * @author Shruti Sahu
 */

#include "CAN.h" /* Include CAN driver header file */

volatile CAN_MSG g_can_rx_msg;  /* Global message structure to store frame from ISR */
volatile u8      g_can_rx_flag = 0; /* Volatile flag set to 1 on receipt */

/**
 * @brief Hardware ISR for CAN1 Receive Interrupt (VIC Channel 26)
 */
void CAN1_Rx_ISR(void) __irq
{
    if(C1GSR & 0x01) /* Check if Receive Buffer Status bit 0 indicates valid message */
    {
        g_can_rx_msg.dlc = (u8)((C1RFS >> 16) & 0x0F); /* Extract 4-bit DLC */
        g_can_rx_msg.rtr = (u8)((C1RFS >> 30) & 0x01); /* Extract 1-bit RTR flag */
        g_can_rx_msg.id  = C1RID;                      /* Extract 11-bit CAN Identifier */
        
        g_can_rx_msg.data[0] = (u8)(C1RDA);       /* Read Data Byte 0 */
        g_can_rx_msg.data[1] = (u8)(C1RDA >> 8);  /* Read Data Byte 1 */
        g_can_rx_msg.data[2] = (u8)(C1RDA >> 16); /* Read Data Byte 2 */
        g_can_rx_msg.data[3] = (u8)(C1RDA >> 24); /* Read Data Byte 3 */
        
        g_can_rx_msg.data[4] = (u8)(C1RDB);       /* Read Data Byte 4 */
        g_can_rx_msg.data[5] = (u8)(C1RDB >> 8);  /* Read Data Byte 5 */
        g_can_rx_msg.data[6] = (u8)(C1RDB >> 16); /* Read Data Byte 6 */
        g_can_rx_msg.data[7] = (u8)(C1RDB >> 24); /* Read Data Byte 7 */
        
        g_can_rx_flag = 1; /* Set volatile message received flag */
        C1CMR = (1 << 2);   /* Release hardware Receive Buffer */
    }
    VICVectAddr = 0; /* Acknowledge End-of-Interrupt to VIC */
}

/**
 * @brief Initializes CAN1 controller hardware and enables CAN Receive Interrupt
 */
void CAN1_Init(void)
{
    PINSEL1 &= ~((3 << 16) | (3 << 18)); /* Clear bits 16-19 of PINSEL1 */
    PINSEL1 |=  ((1 << 16) | (1 << 18)); /* Select RD1 on P0.25 and TD1 on P0.24 */
    
    C1MOD = 0x01;       /* Set CAN1 into Reset Mode */
    AFMR = 0x02;        /* Bypass Acceptance Filter (Accept all frames) */
    C1BTR = 0x001C001D; /* Configure 250 kbps bus speed @ 60 MHz VPBDIV */
    C1IER = 0x01;       /* Enable Receive Interrupt */
    C1MOD = 0x00;       /* Return to Normal Operating Mode */
    
    /* Configure VIC Slot 0 for CAN1 Interrupt (Channel 26) */
    VICVectAddr0 = (u32)CAN1_Rx_ISR;                  /* Store ISR address in Slot 0 */
    VICVectCntl0 = VIC_ENABLE_BIT | VIC_CHANNEL_CAN1;   /* Enable Slot 0 for CAN1 Channel 26 */
    VICIntEnable |= (1 << VIC_CHANNEL_CAN1);           /* Enable CAN1 Interrupt in VIC */
}

/**
 * @brief Transmits a CAN frame using hardware Transmit Buffer 1
 * @param msg Pointer to message structure to transmit
 * @return 1 on success, 0 if buffer is busy
 */
u8 CAN1_Tx(CAN_MSG *msg)
{
    if(C1SR & (1 << 2)) /* Check if Transmit Buffer 1 is free */
    {
        C1TFI1 = ((u32)msg->dlc << 16) | (msg->rtr ? (1 << 30) : 0); /* Pack DLC and RTR */
        C1TID1 = msg->id;                                            /* Store CAN ID */
        
        C1TDA1 = ((u32)msg->data[0]) |
                 ((u32)msg->data[1] << 8) |
                 ((u32)msg->data[2] << 16) |
                 ((u32)msg->data[3] << 24); /* Pack Bytes 0-3 */
                 
        C1TDB1 = ((u32)msg->data[4]) |
                 ((u32)msg->data[5] << 8) |
                 ((u32)msg->data[6] << 16) |
                 ((u32)msg->data[7] << 24); /* Pack Bytes 4-7 */
        
        C1CMR = (1 << 0) | (1 << 5); /* Request Transmission on Buffer 1 */
        return 1;                    /* Return 1 */
    }
    return 0; /* Return 0 if busy */
}

/**
 * @brief Non-blocking read from ISR received message buffer
 * @param msg Destination pointer to store received message
 * @return 1 if new message available, 0 otherwise
 */
u8 CAN1_Rx(CAN_MSG *msg)
{
    if(g_can_rx_flag)
    {
        *msg = *(CAN_MSG *)&g_can_rx_msg; /* Copy message structure */
        g_can_rx_flag = 0;                 /* Clear flag */
        return 1;                          /* Return 1 */
    }
    return 0; /* Return 0 */
}
