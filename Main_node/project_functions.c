/**
 * @file project_functions.c
 * @brief High-Level User Mode Processing with Hardware Interrupt Events and Detailed Line-by-Line Comments
 * @author Shruti Sahu
 */
 
#include "project_functions.h" /* Include high-level project functions header file */
#include "dashboard.h"         /* Include 20x4 LCD dashboard graphics header file */
#include "delays.h"            /* Include delay routines header file */
#include "EINT.h"              /* Include hardware external interrupt driver header file */

/**
 * @brief Transmits window command to Window Node; if disconnected, locks position and shows warning
 * @param action WINDOW_CMD_UP or WINDOW_CMD_DOWN
 * @param current_level Current window position (0 to 8 LEDs)
 * @param fault_flag Pointer to system fault flag
 * @return Retained or updated window position
 */
u8 Process_Window_Control(u8 action, u8 current_level, u8 *fault_flag)
{
    CAN_MSG tx_msg, rx_msg;                /* Declare transmit and receive CAN message structures */
    u16 timeout;                           /* Declare timeout counter variable */
    u8 response_received;                  /* Declare response confirmation flag variable */
    u8 original_position = current_level; /* Save initial window level before attempting movement */
    
    tx_msg.id = CAN_ID_WINDOW_CTRL; /* Set CAN ID to 0x101 (Window Control Command ID) */
    tx_msg.dlc = 1;                 /* Set Data Length Code to 1 byte */
    tx_msg.rtr = 0;                 /* Set RTR bit to 0 (Data Frame) */
    tx_msg.data[0] = action;        /* Set Data Byte 0 to action command (0x01 = UP, 0x02 = DOWN) */
    
    /* 1. Physical Boundary Protection Checks */
    if(action == WINDOW_CMD_UP && current_level >= 8)
    {
        display_window_screen(WINDOW_CMD_UP, 8); /* Display fully closed screen on LCD */
        delay_ms(400);                           /* Pause for 400ms */
        LCD_clear();                             /* Clear LCD screen */
        return 8;                                /* Return level 8 (Already fully closed) */
    }
    if(action == WINDOW_CMD_DOWN && current_level == 0)
    {
        display_window_screen(WINDOW_CMD_DOWN, 0); /* Display fully opened screen on LCD */
        delay_ms(400);                             /* Pause for 400ms */
        LCD_clear();                               /* Clear LCD screen */
        return 0;                                  /* Return level 0 (Already fully open) */
    }
    
    /* 2. Transmit Window Control Frame (0x101) over CAN bus */
    CAN1_Tx(&tx_msg);
    
    /* 3. Wait for Window Node Acknowledgement (0x201) via CAN RX ISR with 500ms Timeout */
    response_received = 0; /* Initialize response flag to 0 */
    for(timeout = 0; timeout < 50; timeout++) /* Loop 50 iterations * 10ms = 500ms timeout */
    {
        if(CAN1_Rx(&rx_msg)) /* Read CAN message received by CAN1_Rx_ISR */
        {
            if(rx_msg.id == CAN_ID_WINDOW_STATUS) /* Check if CAN ID is 0x201 (Window Status ACK) */
            {
                current_level = rx_msg.data[0]; /* Update window level from received ACK payload byte 0 */
                response_received = 1;          /* Set response received flag to 1 */
                *fault_flag = 0;                /* Clear ECU fault flag indicating Window Node healthy */
                break;                          /* Break out of timeout waiting loop */
            }
        }
        delay_ms(10); /* Delay 10ms between checks */
    }
    
    /* 4. Node Disconnection Handling: LOCK position & show error screen */
    if(!response_received)
    {
        *fault_flag = 1;                               /* Set ECU fault indicator flag */
        display_node_error_screen(NODE_TYPE_WINDOW);   /* Display Window Node error screen for 1.8s */
        return original_position;                      /* Position stays locked at previous valid level! */
    }
    
    /* Display updated window position on 20x4 LCD screen */
    display_window_screen(action, current_level);
    delay_ms(200); /* Pause 200ms */
    
    /* 5. Continuous Hold Stepping (While push button switch input is physically held down) */
    while((action == WINDOW_CMD_UP && (IOPIN0 & PIN_SW_WIN_UP) == 0) ||
          (action == WINDOW_CMD_DOWN && (IOPIN0 & PIN_SW_WIN_DOWN) == 0))
    {
        if((action == WINDOW_CMD_UP && current_level < 8) ||
           (action == WINDOW_CMD_DOWN && current_level > 0))
        {
            CAN1_Tx(&tx_msg); /* Re-transmit CAN control frame */
            
            response_received = 0; /* Reset response flag */
            for(timeout = 0; timeout < 30; timeout++) /* 300ms timeout loop */
            {
                if(CAN1_Rx(&rx_msg)) /* Read CAN message received by ISR */
                {
                    if(rx_msg.id == CAN_ID_WINDOW_STATUS) /* Check if status ACK */
                    {
                        current_level = rx_msg.data[0]; /* Update window level */
                        response_received = 1;          /* Set flag */
                        break;                          /* Exit loop */
                    }
                }
                delay_ms(10); /* Delay 10ms */
            }
            
            if(!response_received)
            {
                *fault_flag = 1;                             /* Set fault flag */
                display_node_error_screen(NODE_TYPE_WINDOW); /* Display error screen */
                return current_level;                        /* Lock position at last confirmed level */
            }
            
            display_window_screen(action, current_level); /* Render updated level */
            delay_ms(200);                                 /* Pause 200ms */
        }
        else
        {
            display_window_screen(action, current_level); /* Render limit screen */
            delay_ms(100);                                 /* Pause 100ms */
            break;                                         /* Exit continuous loop */
        }
    }
    
    delay_ms(300); /* Pause 300ms after release */
    LCD_clear();   /* Clear LCD screen buffer */
    return current_level; /* Return updated window level */
}

/**
 * @brief Activates Reverse Mode, receives distance telemetry (0x103), and retains last reading on disconnection
 * @param fault_flag Pointer to system fault flag
 */
void Process_Reverse_Mode(u8 *fault_flag)
{
    CAN_MSG tx_msg, rx_msg;     /* Declare transmit and receive CAN message structures */
    u8 distance_cm = 100;       /* Initialize local distance variable to 100 cm */
    u8 zone = ZONE_SAFE;        /* Initialize local zone variable to SAFE */
    u16 watchdog_counter = 0;   /* Declare signal loss timeout counter */
    
    /* Prepare Reverse Mode ENABLE frame (0x102) */
    tx_msg.id = CAN_ID_REVERSE_ENABLE; /* Set CAN ID to 0x102 */
    tx_msg.dlc = 1;                     /* Set DLC to 1 byte */
    tx_msg.rtr = 0;                     /* Set RTR bit to 0 */
    tx_msg.data[0] = REVERSE_ENABLE;    /* Set payload byte 0 to 0x01 (ENABLE) */
    CAN1_Tx(&tx_msg);                   /* Transmit CAN frame over network */
    
    LCD_clear();        /* Clear LCD screen */
    g_flag_reverse = 0; /* Reset volatile EINT2 hardware interrupt flag */
    
    /* Main loop while Reverse Mode is active */
    while(1)
    {
        /* Check if driver presses EINT2 hardware interrupt switch (P0.15) to toggle/exit reverse mode */
        if(g_flag_reverse || (IOPIN0 & PIN_SW_REVERSE) == 0)
        {
            g_flag_reverse = 0; /* Clear EINT2 interrupt flag */
            delay_ms(50);       /* Short delay */
            break;              /* Exit reverse mode loop */
        }
        
        /* Check for incoming distance telemetry frame (0x103) received by CAN RX ISR */
        if(CAN1_Rx(&rx_msg))
        {
            if(rx_msg.id == CAN_ID_DISTANCE_DATA) /* Check if CAN ID is 0x103 */
            {
                distance_cm = rx_msg.data[0]; /* Read distance in cm from payload byte 0 */
                zone = rx_msg.data[1];        /* Read zone code from payload byte 1 */
                watchdog_counter = 0;         /* Reset signal loss timeout counter to 0 */
                *fault_flag = 0;              /* Clear ECU fault flag indicating Reverse ECU healthy */
                
                display_reverse_radar(distance_cm, zone); /* Render real-time reverse radar screen */
            }
        }
        else
        {
            watchdog_counter++; /* Increment signal loss timeout counter */
            
            /* If no telemetry received for 250ms (5 iterations * 50ms), show last valid reading + [DISC] tag */
            if(watchdog_counter >= 5)
            {
                display_reverse_radar(255, 0); /* Pass special disconnection code 255 */
            }
            
            /* Timeout: 16 iterations * 50ms = 800ms without distance telemetry */
            if(watchdog_counter >= 16)
            {
                *fault_flag = 1;                              /* Set ECU fault indicator */
                display_node_error_screen(NODE_TYPE_REVERSE); /* Display Reverse Node error screen */
                break;                                        /* Safely exit loop to dashboard */
            }
        }
        
        delay_ms(50); /* Pause 50ms (20 Hz polling rate) */
    }
    
    /* Prepare and send Reverse Mode DISABLE frame upon exit */
    tx_msg.data[0] = REVERSE_DISABLE; /* Set payload byte 0 to 0x00 (DISABLE) */
    CAN1_Tx(&tx_msg);                 /* Transmit CAN frame */
    
    LCD_clear(); /* Clear LCD screen */
}
