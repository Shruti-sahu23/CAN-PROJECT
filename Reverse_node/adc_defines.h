#ifndef __ADC_DEFINES_H__
#define __ADC_DEFINES_H__

/* Original Repository ADC Definitions on LPC2129 (AD0.0 on P0.27) */
#define AIN0_PIN_FUNC       (1 << 22)  /* PINSEL1: P0.27 as AD0.0 / AIN0 */

/* ADC Control Register (ADCR) Bit Definitions */
#define ADC_CH0_SEL         (1 << 0)   /* Select Channel 0 (AD0.0 / P0.27) */
#define ADC_CLKDIV_VAL      (14 << 8)  /* Clock Divider */
#define ADC_PDN             (1 << 21)  /* Power-up ADC */
#define ADC_START_NOW       (1 << 24)  /* Start Conversion */

/* ADC Data Register (ADDR) Bit Definitions */
#define ADC_DONE_BIT        (1 << 31)  /* Conversion Complete Flag */
#define ADC_RESULT_MASK     0x03FF     /* 10-bit result */

#endif /* __ADC_DEFINES_H__ */
