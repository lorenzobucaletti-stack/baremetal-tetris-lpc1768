/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  31/12/2025
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
** Created by:          Bucaletti Lorenzo, s353632
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "tetris.h"
#include "timer.h"
#include "GLCD.h"

// #define SPEED_NORMAL 0x017D7840
#define SPEED_NORMAL    0x000F4240  // Normal speed of the pieces
// Peripheral clock = 25 MHz --> 1 second = 25.000.000 CCs = 0x017D7840, this should be used on the real board
// For debugging purposes, the time has been decreased since the emulator is slower than the physical board
// # define SPEED_SOFTDROP 0X00BEBC20
#define SPEED_SOFTDROP  0x0007A120 // Double speed

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

/* Variables for saving the button or joystick previous state (Edge Detection) 
   0 = Button released
   1 = Button pressed
*/
static int J_Left_Pressed = 0;
static int J_Right_Pressed = 0;
static int J_Up_Pressed = 0;
static int J_Down_Pressed = 0;
static int Key2_Pressed = 0;
static int Key1_Pressed = 0;

void RIT_IRQHandler (void)
{					
	
	/* ---------------- KEY1 (P2.11) ----------------- */
	// PAUSE
	if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){	
		if(Key1_Pressed == 0){
			Key1_Pressed = 1;
			
			// Reset status
			if(Tetris_IsStarted() == 0){
				Tetris_SetStarted(1);
				Tetris_SetPause(0);
				// Delete start interface
				GUI_Text(165, 240, (uint8_t *)"        ", Black, Black);
				GUI_Text(165, 260, (uint8_t *)"        ", Black, Black);
				GUI_Text(165, 280, (uint8_t *)"         ", Black, Black);
				// Generate the first block
	      Tetris_SpawnPiece();
				// Draw the block just generated
				DrawCurrentPiece();
				// Reset of the timer
				reset_timer(0);
				init_timer(0, SPEED_NORMAL);
				enable_timer(0);
				
			} 
			// Game mode
			else if(Tetris_HasLost() == 0){
				if(Tetris_IsPaused() == 1){
					// Exit pause mode
					Tetris_SetPause(0);
					// Switch on the timer
					enable_timer(0);
					// Delete "PAUSE"
					GUI_Text(180, 240, (uint8_t *)"     ", Black, Black); 
          DrawCurrentPiece(); // Ridisegna pezzo
				} else {
					// Set pause mode
					Tetris_SetPause(1);
					// Switch off the timer
					disable_timer(0);
					GUI_Text(180, 240, (uint8_t *)"PAUSE", Yellow, Red);
				}
			}
		}
	}
	else{
		Key1_Pressed = 0;
	}
	
	// If the game is over, paused or just started, ignore joystick and buttons inputs
  if (Tetris_HasLost() || Tetris_IsPaused() || Tetris_IsStarted() == 0) {
    LPC_RIT->RICTRL |= 0x1; 
    return;
		}
	/* ---------------- JOISTICK LEFT (P1.27) ---------------- */
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	
		// If the button is now pressed, but wasn't before (falling edge)
		if(J_Left_Pressed == 0){
			// Function move to the left
			Tetris_Move(-1); 
			J_Left_Pressed = 1; // Remember that now is pressed
		}
	}
	else{
		J_Left_Pressed = 0; // Now is released
	}

	/* ---------------- JOISTICK RIGHT (P1.28) ---------------- */
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	
		if(J_Right_Pressed == 0){
			// Function move to the right
			Tetris_Move(1);
			J_Right_Pressed = 1;
		}
	}
	else{
		J_Right_Pressed = 0;
	}

	/* ---------------- JOISTICK UP (P1.29) ------------------- */
	// ROTATE
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	
		if(J_Up_Pressed == 0){
			// Function rotate
			Tetris_Rotate();
			J_Up_Pressed = 1;
		}
	}
	else{
		J_Up_Pressed = 0;
	}

	/* ---------------- JOISTICK DOWN (P1.26) ----------------- */
	// SOFT DROP
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
		if (J_Down_Pressed == 0) {
      // Set the speed to double
      LPC_TIM0->MR0 = SPEED_SOFTDROP; //same code line as in "lib_timer.c" in the function "init_timer"
      J_Down_Pressed = 1;
     }
	}
	else{
		if (J_Down_Pressed == 1) {
      // Restore normal speed
      LPC_TIM0->MR0 = SPEED_NORMAL;
      J_Down_Pressed = 0;
		}
	}

	/* ---------------- KEY2 (P2.12) ----------------- */
	// HARD DROP
	if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){	
		if(Key2_Pressed == 0){
			// Hard drop function
			Tetris_HardDrop();
			Key2_Pressed = 1;
		}
	}
	else{
		Key2_Pressed = 0;
	}

	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
