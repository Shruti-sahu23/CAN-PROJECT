/**
 * @file MAIN_WINDOW_NODE.c
 * @brief Window Glass Control ECU Main Entry Point (Interrupt-Driven Architecture)
 * @author Shruti Sahu
 * 
 * Hardware Connections & Interrupts:
 * - 8 Window Position LEDs: P0.0 to P0.7 (Active LOW)
 * - CAN1 Transceiver (MCP2551): P0.24 (TD1), P0.25 (RD1) with CAN RX Hardware ISR
 */

#include <lpc21xx.h>         /* Include LPC2129 register definitions */
#include "types.h"           /* Include standard data types */
#include "delays.h"          /* Include delay routines */
#include "CAN.h"             /* Include CAN driver header with VIC RX ISR */
#include "window_control.h"  /* Include 8-LED window control driver */

/**
 * @brief Window Node Main Execution Entry Point
 * @return Returns 0
 */
int main(void)
{
    CAN_MSG rx_msg; /* Declare receive CAN message structure */
    
    /* 1. Initialize 8-LED Window Position Hardware GPIOs (P0.0 - P0.7) */
    Window_Control_Init(); /* Configure P0.0 to P0.7 as active-LOW outputs */
    
    /* 2. Initialize Hardware CAN1 Peripheral with CAN RX ISR (250 kbps on P0.24, P0.25) */
    CAN1_Init(); /* Configure C1MOD, C1BTR, C1IER, and VIC slot 0 for CAN1 Receive Interrupt */
    
    /* 3. Main Interrupt Event Handling Loop */
    while(1)
    {
        /* Read message received by CAN RX ISR handler */
        if(CAN1_Rx(&rx_msg))
        {
            if(rx_msg.id == CAN_ID_WINDOW_CTRL) /* Check if CAN ID is 0x101 (Window Control Command) */
            {
                /* Execute UP/DOWN step and transmit status ACK frame (0x201) */
                Window_Execute_Command(rx_msg.data[0]); /* Pass command byte 0x01 (UP) or 0x02 (DOWN) */
            }
        }
        
        delay_ms(10); /* Loop delay of 10 milliseconds */
    }
    
   // return 0; /* Return 0 */
}
