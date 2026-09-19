#ifndef __ADC_H
#define __ADC_H

#include "LPC17xx.h"
#include <string.h>

/* lib_adc.c */
void ADC_init (void);
void ADC_start_conversion (void);

/* IRQ_adc.c */
void ADC_IRQHandler(void);

/* Global variable for speed (Timer Tick) */
extern volatile uint32_t current_base_interval;

#endif
