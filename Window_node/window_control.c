#include "window_control.h"
#include "delays.h"

/* Current Window Position: 0 (All OFF - Fully Open) to 8 (All ON - Fully Closed) */
static u8 WindowPosition = 0;

void Window_Update_LEDs(u8 level)
{
    u32 leds_to_turn_on = 0;
    u8 i;
    
    if(level > WINDOW_MAX_LEVEL)
    {
        level = WINDOW_MAX_LEVEL;
    }
    
    /* Calculate bitmask for active LEDs (0 to level-1) */
    for(i = 0; i < level; i++)
    {
        leds_to_turn_on |= (1 << i);
    }
    
    /* Active LOW LEDs: Clear bits to turn ON, Set bits to turn OFF */
    IOCLR0 = leds_to_turn_on & WINDOW_LED_MASK;
    IOSET0 = (~leds_to_turn_on) & WINDOW_LED_MASK;
}

void Window_Control_Init(void)
{
    /* Configure P0.0 - P0.7 as GPIO Output */
    IODIR0 |= WINDOW_LED_MASK;
    
    /* Initialize with all LEDs OFF (Level 0 - Fully Open) */
    WindowPosition = 0;
    Window_Update_LEDs(WindowPosition);
}

u8 Window_Step_Up(void)
{
    /* Only increment if below upper boundary */
    if(WindowPosition < WINDOW_MAX_LEVEL)
    {
        WindowPosition++;
        Window_Update_LEDs(WindowPosition);
    }
    return WindowPosition;
}

u8 Window_Step_Down(void)
{
    /* Only decrement if above lower boundary */
    if(WindowPosition > 0)
    {
        WindowPosition--;
        Window_Update_LEDs(WindowPosition);
    }
    return WindowPosition;
}

void Window_Execute_Command(u8 cmd)
{
    CAN_MSG status_msg;
    
    if(cmd == WINDOW_CMD_UP)
    {
        Window_Step_Up();
    }
    else if(cmd == WINDOW_CMD_DOWN)
    {
        Window_Step_Down();
    }
    
    /* Send Immediate Telemetry Feedback to Main Node */
    status_msg.id = CAN_ID_WINDOW_STATUS;
    status_msg.dlc = 2;
    status_msg.rtr = 0;
    status_msg.data[0] = WindowPosition;                       /* Exact Level: 0 to 8 */
    status_msg.data[1] = (u8)((WindowPosition * 100) / WINDOW_MAX_LEVEL); /* Percentage: 0% to 100% */
    CAN1_Tx(&status_msg);
}
