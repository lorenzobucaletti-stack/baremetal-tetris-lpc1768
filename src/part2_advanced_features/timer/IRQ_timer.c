/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  31/12/2025
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
** Created by:          Bucaletti Lorenzo, s353632
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "GLCD.h" 
#include "TouchPanel.h"
#include <stdio.h> /*for sprintf*/
#include "tetris.h"

// Sine audio vector
uint16_t SinTable[45] =                                       
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER0_IRQHandler (void)
{
  // Every time the timer is triggered, the piece falls by one block
	Tetris_Update();

  LPC_TIM0->IR = 1;           /* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer2_IRQHandler
** Descriptions:		AUDIO FREQUENZA (Generazione Onda su DAC)
******************************************************************************/
void TIMER2_IRQHandler (void)
{
	static int sineticks=0;
	static int currentValue; 
	
	// DAC management
	currentValue = SinTable[sineticks];
	
	// Writes on DAC (P0.26) - Bit 6-15: Value
	LPC_DAC->DACR = currentValue << 6;
	
	sineticks++;
	if(sineticks==45) sineticks=0;

  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}



/******************************************************************************
** Function name:		Timer3_IRQHandler
** Descriptions:		AUDIO DURATA (Stop Nota)
******************************************************************************/
void TIMER3_IRQHandler (void)
{
	// The given note time has ended: switch off (Timer 2)
	disable_timer(2);
  LPC_TIM3->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
