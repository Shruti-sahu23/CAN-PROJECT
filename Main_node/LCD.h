#ifndef __LCD_H__
#define __LCD_H__
 
#include <lpc21xx.h>
#include "types.h"

/* Original Repository LCD Pinout on LPC2129 */
/* Data Lines D0-D7 connected to P0.0 - P0.7 */
#define LCD_DATA_DIR    (0xFF)        /* P0.0 - P0.7 */
#define LCD_DATA_MASK   (0xFF)
#define LCD_DATA_SHIFT  0

/* Control Pins */
#define LCD_EN          (1 << 8)      /* P0.8 - Enable Strobe */
#define LCD_RS          (1 << 9)      /* P0.9 - Register Select */
#define LCD_RW          (1 << 10)     /* P0.10 - Read / Write (0 = Write) */

/* HD44780 20x4 LCD Line Addresses */
#define LCD_LINE1       0x80
#define LCD_LINE2       0xC0
#define LCD_LINE3       0x94
#define LCD_LINE4       0xD4

/* Custom Character Codes in CGRAM */
#define CHAR_DEGREE     0x00
#define CHAR_UP_ARROW   0x01
#define CHAR_DOWN_ARROW 0x02
#define CHAR_SOLID_BAR  0x03

/* Function Prototypes */
void LCD_init(void);
void LCD_cmd(u8 cmd);
void LCD_data(u8 data);
void LCD_string(const char *str);
void LCD_set_cursor(u8 row, u8 col);
void LCD_clear(void);
void LCD_integer(s32 num);
void LCD_float(f32 val, u8 decimal_places);
void LCD_load_custom_chars(void);

#endif /* __LCD_H__ */
