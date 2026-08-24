#ifndef __ADC_H__
#define __ADC_H__

#include <lpc21xx.h>
#include "types.h"
 
/* Function Prototypes */
void ADC_Init(void);
u16  ADC_Read(void);

#endif /* __ADC_H__ */
