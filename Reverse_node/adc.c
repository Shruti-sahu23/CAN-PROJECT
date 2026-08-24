#include "adc.h"
#include "delays.h"
 
void ADC_Init(void)
{
    /* 1. Configure P0.27 as AD0.0 / AIN0 analog input */
    PINSEL1 &= ~((u32)3 << 22);
    PINSEL1 |=  ((u32)1 << 22);
    
    /* 2. Configure ADCR:
     * - Channel 0 (bit 0 = 1)
     * - CLKDIV = 14 (bits 8..15) -> 60MHz / 15 = 4.0 MHz (<= 4.5 MHz max limit)
     * - PDN = 1 (bit 21 = 1, Operational mode)
     */
    ADCR = (1 << 0) | (14 << 8) | (1 << 21);
    
    delay_ms(2); /* Allow ADC internal reference and analog circuitry to settle */
}

u16 ADC_Read(void)
{
    u32 reg_val;
    
    /* Trigger ADC conversion (START = 001 on bits 24..26) */
    ADCR |= (1 << 24);
    
    /* Wait until conversion completes (DONE bit 31 set) */
    do {
        reg_val = ADDR;
    } while((reg_val & (1U << 31)) == 0);
    
    /* Clear START bits */
    ADCR &= ~(7 << 24);
    
    /* Extract 10-bit Result (bits 6..15) */
    return (u16)((reg_val >> 6) & 0x03FF);
}
