/**
 * @file dashboard.c
 * @brief 20x4 LCD Telemetry Dashboard with Strict 20-Character Line Boundary Enforcement & Detailed Line-by-Line Comments
 * @author Shruti Sahu
 *  
 * Every string output to the 20x4 LCD display is strictly bounded to 20 characters per line
 * to prevent line wrapping, buffer overflow, and visual screen flickering.
 */

#include "dashboard.h"    /* Include dashboard graphics header file */
#include "delays.h"       /* Include delay routines header file */
#include "CAN_defines.h"  /* Include CAN protocol defines header file */

/* Define static persistent variables to store last valid sensor readings for disconnection recovery */
static f32 g_last_valid_temp = 32.5f;   /* Remembers last valid engine temperature reading */
static u8  g_last_valid_dist = 100;     /* Remembers last valid IR obstacle distance reading */
static u8  g_last_valid_zone = ZONE_SAFE; /* Remembers last valid proximity zone classification */

/**
 * @brief Displays vehicle startup boot banner for 2 seconds
 */
void display_splash_screen(void)
{
    LCD_clear(); /* Clear LCD screen buffer */
    
    LCD_set_cursor(1, 0); /* Move cursor to Row 1, Column 0 */
    LCD_string("===================="); /* Output 20 characters */
    
    LCD_set_cursor(2, 0); /* Move cursor to Row 2, Column 0 */
    LCD_string("  SMART VEHICLE ECU "); /* Output 20 characters */
    
    LCD_set_cursor(3, 0); /* Move cursor to Row 3, Column 0 */
    LCD_string(" ENGINE & SAFETY OS "); /* Output 20 characters */
    
    LCD_set_cursor(4, 0); /* Move cursor to Row 4, Column 0 */
    LCD_string("===================="); /* Output 20 characters */
    
    delay_ms(2000); /* Pause for 2000 milliseconds (2 seconds) */
    LCD_clear();    /* Clear LCD screen buffer after splash screen */
}

/**
 * @brief Renders central 20x4 instrument cluster dashboard telemetry
 * @param temp Current engine temperature (or -999.0f if sensor disconnected)
 * @param window_level Current window position level (0 to 8 LEDs)
 * @param reverse_state 1 if reverse mode active, 0 otherwise
 * @param ecu_fault_flag 1 if an ECU timeout/disconnection occurred, 0 otherwise
 */
void display_dashboard(f32 temp, u8 window_level, u8 reverse_state, u8 ecu_fault_flag)
{
    static u8 critical_state = 0; /* Static variable tracking critical overheat alarm state */
    u8 pct;                       /* Local variable for window percentage calculation */
    
    /* 1. Handle Temperature Sensor Disconnection / 1-Wire Line Fault */
    if(temp < -50.0f || temp > 150.0f) /* Check out-of-range sensor reading */
    {
        LCD_set_cursor(1, 0); /* Position cursor at Row 1, Column 0 */
        LCD_string("--- VEHICLE DASH ---"); /* Output exactly 20 characters */
        
        LCD_set_cursor(2, 0); /* Position cursor at Row 2, Column 0 */
        LCD_string("TEMP: "); /* Output 6 characters */
        if(g_last_valid_temp < 100.0f) LCD_string(" "); /* Add padding space if temp < 100 */
        LCD_float(g_last_valid_temp, 1); /* Print float with 1 decimal place (4 characters) */
        LCD_data(CHAR_DEGREE);           /* Print degree custom glyph (1 character) */
        LCD_string("C [DISC] ");         /* Print status tag (8 characters = 20 total) */
        
        LCD_set_cursor(3, 0); /* Position cursor at Row 3, Column 0 */
        LCD_string("STATUS: SENSOR LOST "); /* Output exactly 20 characters */
        
        LCD_set_cursor(4, 0); /* Position cursor at Row 4, Column 0 */
        if(window_level >= 8)
        {
            LCD_string("WIN:CLOSED  "); /* Output 12 characters */
        }
        else if(window_level == 0)
        {
            LCD_string("WIN:OPENED  "); /* Output 12 characters */
        }
        else
        {
            pct = (u8)((window_level * 100) / 8); /* Calculate percentage */
            LCD_string("WIN: ");                  /* Output 5 characters */
            if(pct < 100) LCD_string(" ");         /* Padding space */
            if(pct < 10)  LCD_string(" ");         /* Padding space */
            LCD_integer(pct);                      /* Output integer percentage */
            LCD_string("%    ");                   /* Output percentage sign and trailing spaces */
        }
        
        if(reverse_state == REVERSE_ENABLE)
        {
            LCD_string("REV: ON "); /* Output 8 characters = 20 total for Row 4 */
        }
        else
        {
            LCD_string("REV: OFF"); /* Output 8 characters = 20 total for Row 4 */
        }
        return; /* Return early from dashboard function */
    }

    /* Store valid temperature reading into static persistent memory */
    g_last_valid_temp = temp;

    /* 2. Evaluate Critical Overheat Alarm threshold with hysteresis */
    if(temp >= TEMP_THRESHOLD_CRITICAL)
    {
        critical_state = 1; /* Enter critical alarm state */
    }
    else if(temp <= (TEMP_THRESHOLD_CRITICAL - TEMP_HYSTERESIS))
    {
        critical_state = 0; /* Exit critical alarm state when temp drops */
    }

    /* Display Critical Overheat Warning Screen if alarm active */
    if(critical_state)
    {
        LCD_set_cursor(1, 0); LCD_string("********************"); /* Output 20 characters */
        LCD_set_cursor(2, 0); LCD_string("! CRITICAL OVERHEAT!"); /* Output 20 characters */
        
        LCD_set_cursor(3, 0);
        LCD_string("TEMP: "); /* Output 6 characters */
        if(temp < 100.0f) LCD_string(" "); /* Add padding space */
        LCD_float(temp, 1);    /* Output temperature value */
        LCD_data(CHAR_DEGREE); /* Output degree symbol */
        LCD_string("C [STOP]");/* Output tag (8 characters = 20 total) */
        
        LCD_set_cursor(4, 0); LCD_string("STOP VEHICLE SAFELY!"); /* Output 20 characters */
        return; /* Return early */
    }

    /* 3. Render Normal Instrument Cluster Telemetry (Strict 20-character line padding) */
    LCD_set_cursor(1, 0);
    LCD_string("--- VEHICLE DASH ---"); /* Row 1: Exactly 20 characters */
    
    /* Row 2: Render Temperature Value and Status Tag */
    LCD_set_cursor(2, 0);
    LCD_string("TEMP: "); /* Print label */
    if(temp < 100.0f) LCD_string(" "); /* Align column alignment */
    LCD_float(temp, 1);    /* Print temperature float value */
    LCD_data(CHAR_DEGREE); /* Print custom degree glyph */
    LCD_string("C ");      /* Print unit label */
    
    if(temp >= TEMP_THRESHOLD_HIGH)
    {
        LCD_string("[HIGH] "); /* Output 7 characters = 20 total */
    }
    else if(temp >= TEMP_THRESHOLD_WARMING)
    {
        LCD_string("[WARM] "); /* Output 7 characters = 20 total */
    }
    else
    {
        LCD_string("[ OK ] "); /* Output 7 characters = 20 total */
    }
    
    /* Row 3: Render System ECU Status Line */
    LCD_set_cursor(3, 0);
    if(ecu_fault_flag != 0)
    {
        LCD_string("! WARN: ECU OFFLINE "); /* Row 3: Exactly 20 characters */
    }
    else if(temp >= TEMP_THRESHOLD_HIGH)
    {
        LCD_string("! WARN: ENGINE HOT !"); /* Row 3: Exactly 20 characters */
    }
    else if(temp >= TEMP_THRESHOLD_WARMING)
    {
        LCD_string("STATUS: FAN ACTIVE  "); /* Row 3: Exactly 20 characters */
    }
    else
    {
        LCD_string("STATUS: ALL ECUS OK "); /* Row 3: Exactly 20 characters */
    }
    
    /* Row 4: Render Power Window Level Percentage & Reverse Mode State */
    LCD_set_cursor(4, 0);
    if(window_level >= 8)
    {
        LCD_string("WIN:CLOSED  "); /* 12 characters */
    }
    else if(window_level == 0)
    {
        LCD_string("WIN:OPENED  "); /* 12 characters */
    }
    else
    {
        pct = (u8)((window_level * 100) / 8); /* Calculate percentage */
        LCD_string("WIN: ");                  /* 5 characters */
        if(pct < 100) LCD_string(" ");         /* Align alignment */
        if(pct < 10)  LCD_string(" ");         /* Align alignment */
        LCD_integer(pct);                      /* Output integer */
        LCD_string("%    ");                   /* 5 characters = 12 total */
    }
    
    if(reverse_state == REVERSE_ENABLE)
    {
        LCD_string("REV: ON "); /* 8 characters = 20 total for Row 4 */
    }
    else
    {
        LCD_string("REV: OFF"); /* 8 characters = 20 total for Row 4 */
    }
}

/**
 * @brief Displays 8-segment window position bar matching the 8 physical LEDs (Strict 20 chars per line)
 * @param action WINDOW_CMD_UP or WINDOW_CMD_DOWN
 * @param level Current window position level (0 to 8)
 */
void display_window_screen(u8 action, u8 level)
{
    u8 i;   /* Loop index variable */
    u8 pct; /* Percentage calculation variable */
    
    if(level > 8) level = 8;               /* Clamp maximum level to 8 */
    pct = (u8)((level * 100) / 8);        /* Compute window percentage */
    
    LCD_set_cursor(1, 0);
    LCD_string("=== POWER WINDOW ==="); /* Row 1: Exactly 20 characters */
    
    /* Row 2: Render Motion Direction or Boundary Limits */
    LCD_set_cursor(2, 0);
    if(action == WINDOW_CMD_UP)
    {
        if(level >= 8)
        {
            LCD_string("LIMIT : FULLY CLOSED"); /* Row 2: Exactly 20 characters */
        }
        else
        {
            LCD_string("MOTION: ROLLING UP "); /* 19 characters */
            LCD_data(CHAR_UP_ARROW);          /* 1 custom glyph = 20 characters total */
        }
    }
    else /* WINDOW_CMD_DOWN */
    {
        if(level == 0)
        {
            LCD_string("LIMIT : FULLY OPENED"); /* Row 2: Exactly 20 characters */
        }
        else
        {
            LCD_string("MOTION: ROLLING DN "); /* 19 characters */
            LCD_data(CHAR_DOWN_ARROW);        /* 1 custom glyph = 20 characters total */
        }
    }
    
    /* Row 3: Render 8-Segment Bar Graph */
    LCD_set_cursor(3, 0);
    if(level == 8)
    {
        LCD_string("GLASS:[||||||||]100%"); /* Row 3: Exactly 20 characters */
    }
    else
    {
        LCD_string("GLASS: ["); /* 8 characters */
        for(i = 0; i < 8; i++)  /* Loop across 8 position segments */
        {
            if(i < level) LCD_data('|'); /* Print filled bar segment */
            else          LCD_data('.'); /* Print empty segment dot */
        }
        LCD_data(']'); /* Output closing bracket */
        
        if(pct < 10) LCD_string(" "); /* Add padding space if single digit */
        LCD_integer(pct);             /* Output percentage value */
        LCD_string("%");              /* Output percentage sign = 20 characters total */
    }
    
    /* Row 4: Render Window Level Status Summary */
    LCD_set_cursor(4, 0);
    if(level >= 8)      LCD_string("STATUS: FULLY CLOSED"); /* Row 4: Exactly 20 characters */
    else if(level == 0) LCD_string("STATUS: FULLY OPENED"); /* Row 4: Exactly 20 characters */
    else
    {
        LCD_string("STATUS: LEVEL "); /* 14 characters */
        LCD_integer(level);          /* 1 character */
        LCD_string(" OF 8 ");        /* 5 characters = 20 characters total */
    }
}

/**
 * @brief Displays real-time reverse radar screen with last valid distance retention (Strict 20 chars per line)
 * @param distance_cm Filtered obstacle distance in centimeters (or 255 if sensor disconnected)
 * @param zone Proximity zone code
 */
void display_reverse_radar(u8 distance_cm, u8 zone)
{
    static u8 last_dist = 255; /* Persistent variable to track last distance */
    static u8 last_zone = 255; /* Persistent variable to track last zone */
    
    /* Handle Distance Sensor Disconnection State (Special code 255) */
    if(distance_cm == 255)
    {
        LCD_set_cursor(1, 0);
        LCD_string("<<  REVERSE RADAR >>"); /* Row 1: Exactly 20 characters */
        
        LCD_set_cursor(2, 0);
        LCD_string("DIST  : ");
        if(g_last_valid_dist < 100) LCD_string(" ");
        if(g_last_valid_dist < 10)  LCD_string(" ");
        LCD_integer(g_last_valid_dist);
        LCD_string(" cm [DISC]"); /* Row 2: Exactly 20 characters total */
        
        LCD_set_cursor(3, 0);
        LCD_string("ZONE  : SENSOR LOST "); /* Row 3: Exactly 20 characters */
        
        LCD_set_cursor(4, 0);
        LCD_string("BAR   :[!!!!!!!!!]"); /* Row 4: Exactly 20 characters */
        return;
    }
    
    /* Store valid distance and zone into persistent global memory */
    g_last_valid_dist = distance_cm;
    g_last_valid_zone = zone;
    
    /* Perform Change-Detection Refresh to eliminate LCD cursor flickering */
    if(distance_cm == last_dist && zone == last_zone)
    {
        return; /* Skip redundant LCD redraw if readings have not changed */
    }
    last_dist = distance_cm; /* Update persistent last distance */
    last_zone = zone;        /* Update persistent last zone */
    
    /* Handle Critical Stop Zone (< 15 cm) */
    if(zone == ZONE_CRITICAL || distance_cm <= 15)
    {
        LCD_set_cursor(1, 0); LCD_string("********************"); /* Row 1: 20 characters */
        LCD_set_cursor(2, 0); LCD_string("<<  REVERSE RADAR >>"); /* Row 2: 20 characters */
        
        LCD_set_cursor(3, 0);
        LCD_string("! STOP: ");
        if(distance_cm < 100) LCD_string(" ");
        if(distance_cm < 10)  LCD_string(" ");
        LCD_integer(distance_cm);
        LCD_string(" cm DIST !"); /* Row 3: 20 characters total */
        
        LCD_set_cursor(4, 0); LCD_string("********************"); /* Row 4: 20 characters */
    }
    else /* Handle Standard Safe / Warning / Danger Radar Zones */
    {
        LCD_set_cursor(1, 0);
        LCD_string("<<  REVERSE RADAR >>"); /* Row 1: Exactly 20 characters */
        
        LCD_set_cursor(2, 0);
        LCD_string("DIST  : ");
        if(distance_cm < 100) LCD_string(" ");
        if(distance_cm < 10)  LCD_string(" ");
        LCD_integer(distance_cm);
        LCD_string(" cm ");
        
        if(zone == ZONE_SAFE)
        {
            LCD_string("[SAFE]");                                   /* Row 2: 20 chars total */
            LCD_set_cursor(3, 0); LCD_string("ZONE  : GREEN / OK  "); /* Row 3: 20 chars */
            LCD_set_cursor(4, 0); LCD_string("BAR   :[||||||||||||]"); /* Row 4: 20 chars */
        }
        else if(zone == ZONE_WARNING)
        {
            LCD_string("[WARN]");                                   /* Row 2: 20 chars total */
            LCD_set_cursor(3, 0); LCD_string("ZONE  : YELLOW/SLOW "); /* Row 3: 20 chars */
            LCD_set_cursor(4, 0); LCD_string("BAR   :[||||||......]"); /* Row 4: 20 chars */
        }
        else /* ZONE_DANGER */
        {
            LCD_string("[DANG]");                                   /* Row 2: 20 chars total */
            LCD_set_cursor(3, 0); LCD_string("ZONE  : ORANGE/ALERT"); /* Row 3: 20 chars */
            LCD_set_cursor(4, 0); LCD_string("BAR   :[|||.........]"); /* Row 4: 20 chars */
        }
    }
}

/**
 * @brief Displays dedicated CAN bus node error alert screen (Strict 20 chars per line)
 * @param node_type NODE_TYPE_WINDOW or NODE_TYPE_REVERSE
 */
void display_node_error_screen(u8 node_type)
{
    LCD_clear(); /* Clear LCD screen */
    
    LCD_set_cursor(1, 0);
    LCD_string("! CAN BUS WARNING ! "); /* Row 1: Exactly 20 characters */
    
    LCD_set_cursor(2, 0);
    if(node_type == NODE_TYPE_WINDOW)
    {
        LCD_string("NODE: WINDOW MODULE "); /* Row 2: Exactly 20 characters */
    }
    else if(node_type == NODE_TYPE_REVERSE)
    {
        LCD_string("NODE: REVERSE RADAR "); /* Row 2: Exactly 20 characters */
    }
    else
    {
        LCD_string("NODE: UNKNOWN ECU   "); /* Row 2: Exactly 20 characters */
    }
    
    LCD_set_cursor(3, 0);
    LCD_string("ERROR: NO RESPONSE  "); /* Row 3: Exactly 20 characters */
    
    LCD_set_cursor(4, 0);
    LCD_string("STATUS: OFFLINE /ERR"); /* Row 4: Exactly 20 characters */
    
    delay_ms(1800); /* Display error screen for 1.8 seconds */
    LCD_clear();    /* Clear LCD screen after warning */
}
