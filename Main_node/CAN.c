/**
 * @file CAN.c
 * @brief LPC2129 Hardware CAN1 Controller Driver Implementation with VIC Interrupt ISR
 * @author Shruti Sahu
 */

#include "CAN.h" /* Include CAN driver header file */

/* Define global volatile variables for CAN Receive Interrupt */
volatile CAN_MSG g_can_rx_msg;  /* Global structure to store incoming frame from ISR */
volatile u8      g_can_rx_flag = 0; /* Volatile flag initialized to 0 */

/**
 * @brief Hardware ISR for CAN1 Receive Interrupt (VIC Channel 26)
 */
void CAN1_Rx_ISR(void) __irq
{
    /* Check if Receive Buffer Status (RBS bit 0 in C1GSR) indicates message available */
    if(C1GSR & 0x01)
    {
        /* Extract 4-bit Data Length Code (DLC) from bits 16-19 of C1RFS register */
        g_can_rx_msg.dlc = (u8)((C1RFS >> 16) & 0x0F);
        
        /* Extract 1-bit Remote Transmission Request (RTR) from bit 30 of C1RFS register */
        g_can_rx_msg.rtr = (u8)((C1RFS >> 30) & 0x01);
        
        /* Extract 11-bit Standard Identifier from C1RID register */
        g_can_rx_msg.id  = C1RID;
        
        /* Read Data Byte 0 from lower byte of Data Register A (C1RDA) */
        g_can_rx_msg.data[0] = (u8)(C1RDA);
        
        /* Read Data Byte 1 from second byte of Data Register A (C1RDA) */
        g_can_rx_msg.data[1] = (u8)(C1RDA >> 8);
        
        /* Read Data Byte 2 from third byte of Data Register A (C1RDA) */
        g_can_rx_msg.data[2] = (u8)(C1RDA >> 16);
        
        /* Read Data Byte 3 from fourth byte of Data Register A (C1RDA) */
        g_can_rx_msg.data[3] = (u8)(C1RDA >> 24);
        
        /* Read Data Byte 4 from lower byte of Data Register B (C1RDB) */
        g_can_rx_msg.data[4] = (u8)(C1RDB);
        
        /* Read Data Byte 5 from second byte of Data Register B (C1RDB) */
        g_can_rx_msg.data[5] = (u8)(C1RDB >> 8);
        
        /* Read Data Byte 6 from third byte of Data Register B (C1RDB) */
        g_can_rx_msg.data[6] = (u8)(C1RDB >> 16);
        
        /* Read Data Byte 7 from fourth byte of Data Register B (C1RDB) */
        g_can_rx_msg.data[7] = (u8)(C1RDB >> 24);
        
        /* Set volatile message received flag to 1 */
        g_can_rx_flag = 1;
        
        /* Release Receive Buffer by writing 1 to bit 2 of Command Register (C1CMR) */
        C1CMR = (1 << 2);
    }
    
    /* Acknowledge End-of-Interrupt to VIC by writing 0 to VICVectAddr register */
    VICVectAddr = 0;
}

/**
 * @brief Initializes CAN1 controller hardware and enables CAN Receive Interrupt
 */
void CAN1_Init(void)
{
    /* 1. Configure PINSEL1 register to select P0.25 as RD1 (CAN1 Rx) and P0.24 as TD1 (CAN1 Tx) */
    PINSEL1 &= ~((3 << 16) | (3 << 18)); /* Clear bits 16-19 of PINSEL1 register */
    PINSEL1 |=  ((1 << 16) | (1 << 18)); /* Set bit 16 and bit 18 to select CAN1 functions */
    
    /* 2. Set CAN1 into Reset/Initialization Mode by writing 1 to bit 0 of C1MOD register */
    C1MOD = 0x01;
    
    /* 3. Bypass Acceptance Filter by writing 2 to AFMR register (Accept all CAN messages) */
    AFMR = 0x02;
    
    /* 4. Configure Bus Timing Register (C1BTR) for 250 kbps @ 60 MHz VPBDIV clock */
    C1BTR = 0x001C001D;
    
    /* 5. Enable Receive Interrupt by writing 1 to bit 0 of C1IER register */
    C1IER = 0x01;
    
    /* 6. Return CAN1 to Normal Operating Mode by writing 0 to C1MOD register */
    C1MOD = 0x00;
    
    /* 7. Configure VIC Slot 3 for CAN1 Interrupt (Channel 26) */
    VICVectAddr3 = (u32)CAN1_Rx_ISR;                  /* Store ISR function address into Slot 3 */
    VICVectCntl3 = VIC_ENABLE_BIT | VIC_CHANNEL_CAN1;   /* Enable Slot 3 and map to VIC Channel 26 */
    VICIntEnable |= (1 << VIC_CHANNEL_CAN1);           /* Enable CAN1 Interrupt Channel 26 in VIC */
}

/**
 * @brief Transmits a CAN frame using hardware Transmit Buffer 1
 * @param msg Pointer to message structure containing ID, DLC, and payload bytes
 * @return 1 on successful transmission request, 0 if buffer is busy
 */
u8 CAN1_Tx(CAN_MSG *msg)
{
    /* Check if Transmit Buffer 1 is free by testing bit 2 of C1SR register */
    if(C1SR & (1 << 2))
    {
        /* Pack DLC and RTR flag into Frame Information Register (C1TFI1) */
        C1TFI1 = ((u32)msg->dlc << 16) | (msg->rtr ? (1 << 30) : 0);
        
        /* Store 11-bit Identifier into Transmit Identifier Register (C1TID1) */
        C1TID1 = msg->id;
        
        /* Pack Data Bytes 0-3 into Transmit Data Register A (C1TDA1) */
        C1TDA1 = ((u32)msg->data[0]) |
                 ((u32)msg->data[1] << 8) |
                 ((u32)msg->data[2] << 16) |
                 ((u32)msg->data[3] << 24);
                 
        /* Pack Data Bytes 4-7 into Transmit Data Register B (C1TDB1) */
        C1TDB1 = ((u32)msg->data[4]) |
                 ((u32)msg->data[5] << 8) |
                 ((u32)msg->data[6] << 16) |
                 ((u32)msg->data[7] << 24);
        
        /* Request transmission on Buffer 1 by setting TR1 bit 0 & Select Tx1 bit 5 in C1CMR */
        C1CMR = (1 << 0) | (1 << 5);
        
        return 1; /* Return 1 indicating successful transmission request */
    }
    
    return 0; /* Return 0 if transmit buffer was busy */
}

/**
 * @brief Non-blocking read from the ISR received message buffer
 * @param msg Destination pointer to store received message
 * @return 1 if message was received by ISR, 0 otherwise
 */
u8 CAN1_Rx(CAN_MSG *msg)
{
    /* Check if volatile message received flag is set */
    if(g_can_rx_flag)
    {
        *msg = *(CAN_MSG *)&g_can_rx_msg; /* Copy received message structure */
        g_can_rx_flag = 0;                 /* Clear flag after reading */
        return 1;                          /* Return 1 indicating new message copied */
    }
    
    return 0; /* Return 0 if no new message available */
}
