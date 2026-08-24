/**
 * @file MAIN_NODE.c
 * @brief Master Instrument Cluster ECU Main Execution Entry Point (Interrupt-Driven Architecture)
 * @author Shruti Sahu
 *  
 * Hardware Connections & Interrupt Mapping:
 * - 20x4 LCD Display: P0.0 to P0.7 (Data), P0.8 (EN), P0.9 (RS), P0.10 (RW)
 * - DS18B20 1-Wire Temp Sensor: P0.22 (DQ)
 * - EINT0 ISR (P0.16): Window UP Hardware Interrupt (VIC Slot 0, Channel 14)
 * - EINT1 ISR (P0.14): Window DOWN Hardware Interrupt (VIC Slot 1, Channel 15)
 * - EINT2 ISR (P0.15): Reverse Mode Hardware Interrupt (VIC Slot 2, Channel 16)
 * - CAN1 RX ISR (P0.24, P0.25): Hardware CAN Receive Interrupt (VIC Slot 3, Channel 26)
 */

#include <lpc21xx.h>         /* Include LPC2129 register definitions */
#include "types.h"           /* Include standard data types */
#include "delays.h"          /* Include calibrated delay routines */
#include "LCD.h"             /* Include 20x4 LCD driver header */
#include "DS18B20.h"         /* Include 1-Wire temperature sensor driver */
#include "CAN.h"             /* Include CAN1 driver header with VIC RX ISR */
#include "dashboard.h"       /* Include 20x4 LCD dashboard graphics driver */
#include "EINT.h"            /* Include hardware external interrupt driver header */
#include "project_functions.h"/* Include high-level user mode functions */

/**
 * @brief Master ECU Main Program Entry Point
 * @return Returns 0 (Standard C entry signature)
 */
int main(void)
{
    f32 engine_temp = 32.5f;     /* Declare and initialize default engine temperature variable */
    f32 read_val;                /* Declare temporary variable to hold raw DS18B20 sensor reading */
    u8 window_current_level = 0; /* Window position level variable initialized to 0 (Fully Open) */
    u8 ecu_fault_flag = 0;       /* System ECU fault indicator flag (0 = Healthy, 1 = ECU Offline) */
    
    /* 1. Initialize 20x4 Alphanumeric LCD Display Interface (P0.0 to P0.10) */
    LCD_init(); /* Call LCD driver initialization routine */
    
    /* 2. Display Automotive Boot Splash Screen for 2 seconds */
    display_splash_screen(); /* Render system boot graphic */
    
    /* 3. Initialize LPC2129 VIC Hardware External Interrupts (EINT0 on P0.16, EINT1 on P0.14, EINT2 on P0.15) */
    External_Interrupts_Init(); /* Configure PINSEL, EXTMODE, EXTPOLAR, and VIC slots 0, 1, 2 */
    
    /* 4. Initialize Hardware CAN1 Controller Peripheral with CAN RX ISR (250 kbps on P0.24, P0.25) */
    CAN1_Init(); /* Configure C1MOD, C1BTR, C1IER, and VIC slot 3 for CAN1 Receive Interrupt */
    
    /* 5. Main Interrupt-Driven Execution Event Loop */
    while(1)
    {
        /* Check EINT0 Hardware Interrupt Event Flag (Triggered by EINT0 ISR on P0.16 press) */
        if(g_flag_win_up)
        {
            g_flag_win_up = 0; /* Clear volatile hardware interrupt event flag */
            window_current_level = Process_Window_Control(WINDOW_CMD_UP, window_current_level, &ecu_fault_flag); /* Execute UP step */
        }
        
        /* Check EINT1 Hardware Interrupt Event Flag (Triggered by EINT1 ISR on P0.14 press) */
        if(g_flag_win_down)
        {
            g_flag_win_down = 0; /* Clear volatile hardware interrupt event flag */
            window_current_level = Process_Window_Control(WINDOW_CMD_DOWN, window_current_level, &ecu_fault_flag); /* Execute DOWN step */
        }
        
        /* Check EINT2 Hardware Interrupt Event Flag (Triggered by EINT2 ISR on P0.15 press) */
        if(g_flag_reverse)
        {
            g_flag_reverse = 0; /* Clear volatile hardware interrupt event flag */
            Process_Reverse_Mode(&ecu_fault_flag); /* Enter Reverse Proximity Radar mode */
        }
        
        /* Sample Engine Temperature from DS18B20 1-Wire Digital Sensor on P0.22 */
        read_val = DS18B20_ReadTemperature(); /* Read temperature from 1-Wire sensor */
        if(read_val > -50.0f && read_val < 150.0f) /* Check if reading is within valid physical range */
        {
            engine_temp = read_val; /* Store valid temperature value */
        }
        else
        {
            engine_temp = -999.0f; /* Flag 1-Wire sensor disconnection error */
        }
        
        /* Render Real-Time 20x4 LCD Telemetry Dashboard (Strictly 20 characters per line max) */
        display_dashboard(engine_temp, window_current_level, REVERSE_DISABLE, ecu_fault_flag);
        
        delay_ms(150); /* Main loop refresh delay of 150 milliseconds */
    }
    
    //return 0; /* Return 0 */
}
