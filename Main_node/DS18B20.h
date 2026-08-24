#ifndef __DS18B20_H__
#define __DS18B20_H__
 
#include <lpc21xx.h>
#include "types.h"

/* DS18B20 1-Wire Pin Assignment (Original Repo: P0.22) */
#define DS18B20_PIN     (1 << 22) /* P0.22 */

/* DS18B20 ROM & Function Commands */
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

/* Function Prototypes */
u8   DS18B20_Init(void);
void DS18B20_WriteByte(u8 byte);
u8   DS18B20_ReadByte(void);
f32  DS18B20_ReadTemperature(void);

#endif /* __DS18B20_H__ */
