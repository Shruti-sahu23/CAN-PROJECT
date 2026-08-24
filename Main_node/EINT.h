/**
 * @file EINT.h
 * @brief LPC2129 Hardware External Interrupt (VIC ISR) Driver Header File
 * @author Shruti Sahu
 * 
 * Hardware Pin & Interrupt Mapping:
 * - P0.16 (EINT0) -> Window UP Button Interrupt (VIC Channel 14, Slot 0)
 * - P0.14 (EINT1) -> Window DOWN Button Interrupt (VIC Channel 15, Slot 1)
 * - P0.15 (EINT2) -> Reverse Gear Mode Switch Interrupt (VIC Channel 16, Slot 2)
 */

#ifndef __EINT_H__ /* Prevent multiple inclusions of header file */
#define __EINT_H__ /* Define header guard token */

#include <lpc21xx.h> /* Include LPC2129 register definitions */
#include "types.h"   /* Include custom fixed-width data types */

/*====================================================================
 *                VIC INTERRUPT CHANNEL CONSTANTS
 *====================================================================*/
#define VIC_CHANNEL_EINT0   14 /* VIC Interrupt Source Channel 14 for EINT0 */
#define VIC_CHANNEL_EINT1   15 /* VIC Interrupt Source Channel 15 for EINT1 */
#define VIC_CHANNEL_EINT2   16 /* VIC Interrupt Source Channel 16 for EINT2 */

#define VIC_ENABLE_BIT      (1 << 5) /* Bit 5 of VICVectCntl register enables vector slot */

/*====================================================================
 *            HARDWARE SWITCH PIN MAPPING DEFINITIONS
 *====================================================================*/
#define PIN_SW_WIN_UP       (1 << 16) /* Pin P0.16 mapped to EINT0 (Window UP Switch) */
#define PIN_SW_WIN_DOWN     (1 << 14) /* Pin P0.14 mapped to EINT1 (Window DOWN Switch) */
#define PIN_SW_REVERSE      (1 << 15) /* Pin P0.15 mapped to EINT2 (Reverse Mode Switch) */

/*====================================================================
 *           VOLATILE INTERRUPT EVENT FLAGS (SET BY ISRs)
 *====================================================================*/
extern volatile u8 g_flag_win_up;   /* Interrupt event flag for Window UP button */
extern volatile u8 g_flag_win_down; /* Interrupt event flag for Window DOWN button */
extern volatile u8 g_flag_reverse;  /* Interrupt event flag for Reverse Mode button */

/*====================================================================
 *                  FUNCTION PROTOTYPES
 *====================================================================*/
void External_Interrupts_Init(void); /* Function prototype to configure hardware interrupts */
void EINT0_Handler(void) __irq;      /* Hardware ISR handler prototype for EINT0 (P0.16) */
void EINT1_Handler(void) __irq;      /* Hardware ISR handler prototype for EINT1 (P0.14) */
void EINT2_Handler(void) __irq;      /* Hardware ISR handler prototype for EINT2 (P0.15) */

#endif /* __EINT_H__ End of header guard */
