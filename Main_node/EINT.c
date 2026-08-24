/**
 * @file EINT.c
 * @brief LPC2129 Hardware External Interrupt (VIC ISR) Driver Implementation
 * @author Shruti Sahu
 *  
 * Hardware Pin & Interrupt Mapping:
 * - P0.16 -> EINT0 (VIC Channel 14, Vector Slot 0): Window UP Button Interrupt
 * - P0.14 -> EINT1 (VIC Channel 15, Vector Slot 1): Window DOWN Button Interrupt
 * - P0.15 -> EINT2 (VIC Channel 16, Vector Slot 2): Reverse Gear Mode Switch Interrupt
 */

#include "EINT.h" /* Include external interrupt header file */

/* Declare global volatile interrupt event flags */
volatile u8 g_flag_win_up = 0;   /* Global flag set by EINT0 ISR on P0.16 press */
volatile u8 g_flag_win_down = 0; /* Global flag set by EINT1 ISR on P0.14 press */
volatile u8 g_flag_reverse = 0;  /* Global flag set by EINT2 ISR on P0.15 press */

/**
 * @brief Hardware ISR for EINT0 (Pin P0.16 - Window UP Switch Interrupt)
 */
void EINT0_Handler(void) __irq
{
    EXTINT = (1 << 0); /* Clear EINT0 hardware interrupt flag by writing 1 to bit 0 */
    g_flag_win_up = 1; /* Set volatile global flag indicating Window UP button was pressed */
    VICVectAddr = 0;   /* Write 0 to VICVectAddr register to signal End-of-Interrupt to VIC */
}

/**
 * @brief Hardware ISR for EINT1 (Pin P0.14 - Window DOWN Switch Interrupt)
 */
void EINT1_Handler(void) __irq
{
    EXTINT = (1 << 1);   /* Clear EINT1 hardware interrupt flag by writing 1 to bit 1 */
    g_flag_win_down = 1; /* Set volatile global flag indicating Window DOWN button was pressed */
    VICVectAddr = 0;     /* Write 0 to VICVectAddr register to signal End-of-Interrupt to VIC */
}

/**
 * @brief Hardware ISR for EINT2 (Pin P0.15 - Reverse Gear Switch Interrupt)
 */
void EINT2_Handler(void) __irq
{
    EXTINT = (1 << 2);  /* Clear EINT2 hardware interrupt flag by writing 1 to bit 2 */
    g_flag_reverse = 1; /* Set volatile global flag indicating Reverse Mode switch was pressed */
    VICVectAddr = 0;    /* Write 0 to VICVectAddr register to signal End-of-Interrupt to VIC */
}

/**
 * @brief Initializes LPC2129 External Interrupt Hardware (EINT0, EINT1, EINT2) and VIC Slots
 */
void External_Interrupts_Init(void)
{
    /* 1. Configure PINSEL registers to assign P0.16 -> EINT0, P0.14 -> EINT1, P0.15 -> EINT2 */
    PINSEL0 &= ~((3U << 28) | (3U << 30)); /* Clear bits 28-31 of PINSEL0 register */
    PINSEL0 |=  ((3U << 28) | (3U << 30)); /* Set bits 28-31 to select EINT1 on P0.14 & EINT2 on P0.15 */
    
    PINSEL1 &= ~(3U << 0); /* Clear bits 0-1 of PINSEL1 register */
    PINSEL1 |=  (1U << 0); /* Set bit 0 to select EINT0 on P0.16 */
    
    /* 2. Configure EXTMODE register to select Edge-Sensitive Mode for EINT0, EINT1, EINT2 */
    EXTMODE |= (1 << 0) | (1 << 1) | (1 << 2); /* Set bits 0, 1, 2 to enable edge trigger mode */
    
    /* 3. Configure EXTPOLAR register to select Falling-Edge Triggering (Active LOW buttons) */
    EXTPOLAR &= ~((1 << 0) | (1 << 1) | (1 << 2)); /* Clear bits 0, 1, 2 for falling edge sensitivity */
    
    /* 4. Clear any existing pending peripheral interrupt flags in EXTINT register */
    EXTINT = (1 << 0) | (1 << 1) | (1 << 2); /* Write 1 to clear pending interrupt flags */
    
    /* 5. Configure Vectored Interrupt Controller (VIC) Slot 0 for EINT0 (P0.16) */
    VICVectAddr0 = (u32)EINT0_Handler;                /* Store ISR handler function address into Slot 0 */
    VICVectCntl0 = VIC_ENABLE_BIT | VIC_CHANNEL_EINT0; /* Enable Slot 0 and map to VIC Channel 14 (EINT0) */
    
    /* 6. Configure Vectored Interrupt Controller (VIC) Slot 1 for EINT1 (P0.14) */
    VICVectAddr1 = (u32)EINT1_Handler;                /* Store ISR handler function address into Slot 1 */
    VICVectCntl1 = VIC_ENABLE_BIT | VIC_CHANNEL_EINT1; /* Enable Slot 1 and map to VIC Channel 15 (EINT1) */
    
    /* 7. Configure Vectored Interrupt Controller (VIC) Slot 2 for EINT2 (P0.15) */
    VICVectAddr2 = (u32)EINT2_Handler;                /* Store ISR handler function address into Slot 2 */
    VICVectCntl2 = VIC_ENABLE_BIT | VIC_CHANNEL_EINT2; /* Enable Slot 2 and map to VIC Channel 16 (EINT2) */
    
    /* 8. Enable Interrupt Channels in VICIntEnable register */
    VICIntEnable |= (1 << VIC_CHANNEL_EINT0) | /* Enable EINT0 Channel 14 in VIC */
                    (1 << VIC_CHANNEL_EINT1) | /* Enable EINT1 Channel 15 in VIC */
                    (1 << VIC_CHANNEL_EINT2);  /* Enable EINT2 Channel 16 in VIC */
}
