#include "LCD.h"
#include "delays.h"

/* Custom Character Bitmaps (5x8 dots) */
static const u8 custom_chars[4][8] = {
    /* 0: Degree Symbol (°) */
    {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00},
    /* 1: Up Arrow (▲) */
    {0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00},
    /* 2: Down Arrow (▼) */
    {0x04, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04, 0x00},
    /* 3: Solid Block (█) */
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
};

void LCD_cmd(u8 cmd)
{
    IOCLR0 = LCD_DATA_MASK;
    IOCLR0 = LCD_RS; /* RS = 0 for Command */
    IOCLR0 = LCD_RW; /* RW = 0 for Write */
    
    IOSET0 = (u32)cmd & LCD_DATA_MASK;
    
    IOSET0 = LCD_EN; /* High-to-Low Pulse on Enable */
    delay_us(10);
    IOCLR0 = LCD_EN;
    delay_ms(2);
}

void LCD_data(u8 data)
{
    IOCLR0 = LCD_DATA_MASK;
    IOSET0 = LCD_RS; /* RS = 1 for Data */
    IOCLR0 = LCD_RW; /* RW = 0 for Write */
    
    IOSET0 = (u32)data & LCD_DATA_MASK;
    
    IOSET0 = LCD_EN; /* High-to-Low Pulse on Enable */
    delay_us(10);
    IOCLR0 = LCD_EN;
    delay_ms(2);
}

void LCD_init(void)
{
    /* Configure LCD GPIO pins P0.0-P0.10 as Output */
    IODIR0 |= (LCD_DATA_DIR | LCD_RS | LCD_RW | LCD_EN);
    
    delay_ms(20);
    LCD_cmd(0x38); /* 8-bit mode, 2 lines / 5x8 font */
    delay_ms(5);
    LCD_cmd(0x38);
    delay_ms(1);
    LCD_cmd(0x38);
    
    LCD_cmd(0x0C); /* Display ON, Cursor OFF */
    LCD_cmd(0x06); /* Entry mode: Increment cursor */
    LCD_cmd(0x01); /* Clear display */
    delay_ms(2);
    
    /* Load custom characters into CGRAM */
    LCD_load_custom_chars();
}

void LCD_load_custom_chars(void)
{
    u8 i, j;
    LCD_cmd(0x40); /* Set CGRAM address to 0x00 */
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 8; j++)
        {
            LCD_data(custom_chars[i][j]);
        }
    }
    LCD_cmd(LCD_LINE1); /* Return to DDRAM line 1 */
}

void LCD_clear(void)
{
    LCD_cmd(0x01);
    delay_ms(2);
}

void LCD_set_cursor(u8 row, u8 col)
{
    u8 addr;
    switch(row)
    {
        case 1: addr = LCD_LINE1 + col; break;
        case 2: addr = LCD_LINE2 + col; break;
        case 3: addr = LCD_LINE3 + col; break;
        case 4: addr = LCD_LINE4 + col; break;
        default: addr = LCD_LINE1; break;
    }
    LCD_cmd(addr);
}

void LCD_string(const char *str)
{
    while(*str)
    {
        LCD_data((u8)*str);
        str++;
    }
}

void LCD_integer(s32 num)
{
    char buf[12];
    int i = 0;
    u8 is_neg = 0;
    
    if(num == 0)
    {
        LCD_data('0');
        return;
    }
    
    if(num < 0)
    {
        is_neg = 1;
        num = -num;
    }
    
    while(num > 0)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    if(is_neg)
    {
        LCD_data('-');
    }
    
    while(i > 0)
    {
        LCD_data(buf[--i]);
    }
}

void LCD_float(f32 val, u8 decimal_places)
{
    s32 int_part;
    f32 frac_part;
    u8 i;
    
    if(val < 0.0f)
    {
        LCD_data('-');
        val = -val;
    }
    
    int_part = (s32)val;
    frac_part = val - (f32)int_part;
    
    LCD_integer(int_part);
    
    if(decimal_places > 0)
    {
        LCD_data('.');
        for(i = 0; i < decimal_places; i++)
        {
            frac_part *= 10.0f;
            int_part = (s32)frac_part;
            LCD_data(int_part + '0');
            frac_part -= (f32)int_part;
        }
    }
}
