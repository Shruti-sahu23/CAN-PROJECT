#include "DS18B20.h"
#include "delays.h"

/*
 * Bit-Banged 1-Wire Protocol for DS18B20 Temperature Sensor (P0.22)
 */

static void DS18B20_PinOutput(void)
{
    IODIR0 |= DS18B20_PIN;
}

static void DS18B20_PinInput(void)
{
    IODIR0 &= ~DS18B20_PIN;
}

static void DS18B20_PinLow(void)
{
    IOCLR0 = DS18B20_PIN;
}

static void DS18B20_PinHigh(void)
{
    IOSET0 = DS18B20_PIN;
}

static u8 DS18B20_PinRead(void)
{
    return ((IOPIN0 & DS18B20_PIN) ? 1 : 0);
}

u8 DS18B20_Init(void)
{
    u8 presence = 0;
    
    DS18B20_PinOutput();
    DS18B20_PinLow();
    delay_us(480); /* Master reset pulse */
    
    DS18B20_PinInput();
    delay_us(70);  /* Wait for DS18B20 presence pulse */
    
    if(!DS18B20_PinRead())
    {
        presence = 1; /* Presence detected */
    }
    
    delay_us(410); /* Complete the initialization cycle */
    return presence;
}

void DS18B20_WriteByte(u8 byte)
{
    u8 i;
    for(i = 0; i < 8; i++)
    {
        DS18B20_PinOutput();
        DS18B20_PinLow();
        
        if(byte & (1 << i))
        {
            /* Write '1' slot */
            delay_us(5);
            DS18B20_PinInput();
            delay_us(55);
        }
        else
        {
            /* Write '0' slot */
            delay_us(60);
            DS18B20_PinInput();
            delay_us(5);
        }
        delay_us(2);
    }
}

u8 DS18B20_ReadByte(void)
{
    u8 i;
    u8 byte = 0;
    
    for(i = 0; i < 8; i++)
    {
        DS18B20_PinOutput();
        DS18B20_PinLow();
        delay_us(2);
        
        DS18B20_PinInput();
        delay_us(10);
        
        if(DS18B20_PinRead())
        {
            byte |= (1 << i);
        }
        delay_us(50);
    }
    
    return byte;
}

f32 DS18B20_ReadTemperature(void)
{
    u8 temp_lsb, temp_msb;
    s16 raw_temp;
    f32 temperature;
    
    /* Issue Temperature Conversion */
    if(!DS18B20_Init())
    {
        return -999.0f; /* Sensor not detected */
    }
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);
    
    delay_ms(15); /* Conversion delay */
    
    /* Read Temperature Scratchpad */
    if(!DS18B20_Init())
    {
        return -999.0f;
    }
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);
    
    temp_lsb = DS18B20_ReadByte();
    temp_msb = DS18B20_ReadByte();
    
    raw_temp = (s16)((temp_msb << 8) | temp_lsb);
    temperature = (f32)raw_temp * 0.0625f;
    
    return temperature;
}
