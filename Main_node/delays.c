#include "delays.h"

/*
 * Accurate Software Delay Functions for LPC2129 (60 MHz CCLK)
 */

void delay_us(u32 us)
{
    u32 i;
    for(; us > 0; us--)
    {
        for(i = 0; i < 10; i++)
        {
            __asm("NOP");
        }
    }
}

void delay_ms(u32 ms)
{
    u32 i;
    for(; ms > 0; ms--)
    {
        for(i = 0; i < 10000; i++)
        {
            __asm("NOP");
        }
    }
}
