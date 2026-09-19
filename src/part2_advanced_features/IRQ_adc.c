/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "adc.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

/* Variabile globale per la velocità corrente (senza soft drop) */
/* Default: 1 secondo (25.000.000 tick @ 25MHz) */
volatile uint32_t current_base_interval = 0x017D7840;

void ADC_IRQHandler(void) {
  	
 uint32_t ad_value;
    
 // Read the converted value (Bit 4-15 contain the result on 12 bit)
 ad_value = ((LPC_ADC->ADGDR >> 4) & 0xFFF);
 
 // SPEED COMPUTATION
 // ADC Range: 0 (Min) -> 4095 (Max)
 // Speed Range: 1 block/s -> 5 block/s
 // Tick Range: 25.000.000 -> 5.000.000 
 // Delta: 20.000.000 tick
 
 // Formula: The more I increase the potentiometer, the more ticks I subtract (becomes faster)
 
 current_base_interval = 25000000 - (int)((ad_value * 20000000.0) / 0xFFF);
 
}
