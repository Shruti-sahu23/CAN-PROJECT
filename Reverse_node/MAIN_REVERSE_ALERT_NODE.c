/**
 * @file MAIN_REVERSE_ALERT_NODE.c
 * @brief Reverse Proximity ECU Main Entry Point (Interrupt-Driven Architecture)
 * @author Shruti Sahu
 * 
 * Hardware Connections & Interrupts:
 * - Sharp GP2D12 IR Sensor: P0.27 (AD0.0 / AIN0)
 * - CAN1 Transceiver (MCP2551): P0.24 (TD1), P0.25 (RD1) with CAN RX Hardware ISR
 */
 
#include <lpc21xx.h>        /* Include LPC2129 register definitions */
#include "types.h"          /* Include standard data types */
#include "delays.h"         /* Include delay routines */
#include "CAN.h"            /* Include CAN driver header with VIC RX ISR */
#include "distance_sensor.h"/* Include Sharp GP2D12 IR distance sensor driver */

/**
 * @brief Reverse Node Main Program Entry Point
 * @return Returns 0
 */
int main(void)
{
    CAN_MSG rx_msg, tx_msg;     /* Declare receive and transmit CAN message structures */
    u8 reverse_mode_active = 0; /* Declare reverse mode state variable initialized to 0 */
    u8 dist_cm;                 /* Declare obstacle distance variable in centimeters */
    u8 zone;                    /* Declare ADAS proximity zone ID variable */
    
    /* 1. Initialize Distance Sensor ADC (P0.27) and Digital Filter Pipeline */
    Distance_Sensor_Init(); /* Configure ADC clock divider and initialize filter buffers */
    
    /* 2. Initialize Hardware CAN1 Peripheral with CAN RX ISR (250 kbps on P0.24, P0.25) */
    CAN1_Init(); /* Configure C1MOD, C1BTR, C1IER, and VIC slot 0 for CAN1 Receive Interrupt */
    
    /* Setup static fields for periodic distance telemetry broadcast frame (0x103) */
    tx_msg.id = CAN_ID_DISTANCE_DATA; /* Set CAN ID to 0x103 (Distance Telemetry ID) */
    tx_msg.dlc = 2;                    /* Set DLC to 2 bytes */
    tx_msg.rtr = 0;                    /* Set RTR bit to 0 (Data Frame) */
    
    /* 3. Main Interrupt Event Servicing Loop */
    while(1)
    {
        /* Read control message received by CAN RX ISR handler */
        if(CAN1_Rx(&rx_msg))
        {
            if(rx_msg.id == CAN_ID_REVERSE_ENABLE) /* Check if CAN ID is 0x102 (Reverse Mode Toggle) */
            {
                reverse_mode_active = rx_msg.data[0]; /* Update mode state (0x01 = ENABLE, 0x00 = DISABLE) */
            }
        }
        
        /* If Reverse Mode is Engaged, sample sensor and stream CAN telemetry at 20 Hz */
        if(reverse_mode_active == REVERSE_ENABLE)
        {
            dist_cm = Distance_Calculate_cm();       /* Sample ADC & run 20-sample median + EMA filter */
            zone    = Distance_Determine_Zone(dist_cm); /* Classify ADAS proximity zone */
            
            tx_msg.data[0] = dist_cm; /**< Byte 0: Filtered Distance in cm */
            tx_msg.data[1] = zone;    /**< Byte 1: ADAS Proximity Zone ID */
            
            CAN1_Tx(&tx_msg); /* Transmit CAN telemetry frame 0x103 over bus */
            
            delay_ms(50); /* Pause 50ms (20 Hz telemetry streaming rate) */
        }
        else
        {
            delay_ms(20); /* Low-power idle state when reverse gear is disengaged */
        }
    }
    
    //return 0; /* Return 0 */
}
