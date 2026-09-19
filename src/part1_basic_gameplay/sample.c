/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           sample.c
** Last modified Date:  31/12/2025
** Last Version:        1.0
** Descriptions:        Logic management of Tetris implementation
**--------------------------------------------------------------------------------------------------------
** Created by:          Bucaletti Lorenzo, s353632
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD.h" 
#include "TouchPanel.h"
#include "timer.h"
#include "RIT.h"
#include "tetris.h"
#include <stdio.h>

//#define SPEED_NORMAL 0x017D7840
#define SPEED_NORMAL    0x000F4240  // Normal speed of the pieces
// Peripheral clock = 25 MHz --> 1 second = 25.000.000 CCs = 0x017D7840, this should be used on the real board
// init_timer(0, 0x017D7840);
// For debugging purposes, the time has been decreased since the emulator is slower than the physical board

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int main(void)
{
	// System initialization
  SystemInit();  // Clock to 100MHz (CCLK)
  // LCD display initialization
  LCD_Initialization();
	
  // Draw the initial graphics of the game
  Init_Tetris(); 
	
	// RIT initialization (joystick and buttons)
  init_RIT(0x004C4B40); 
  enable_RIT();
	
	// Timer0 intialization (for the pieces fall)
	// Peripheral clock = 25 MHz --> 1 second = 25.000.000 CCs = 0x017D7840, this should be used on the real board
  // init_timer(0, 0x017D7840);
  // For debugging purposes, the time has been decreased since the emulator is slower than the physical board
	init_timer(0, SPEED_NORMAL); // 0x0016E360 = 1.500.000
	
	// The game starts in pause mode
	disable_timer(0);
	
  // Infinite loop and power-saving mode
  while (1) {
		// if the game has finished
		if(Tetris_HasLost() == 1){
			// Update the highscore
			Tetris_UpdateHighScore();
			// Print final stats of the game
			PrintStats();
			
			// Initialazing a new game
			Init_Tetris();
			
			// Disable timer to start the new game in pause mode
			disable_timer(0);
			// Reset normal speed
			init_timer(0, SPEED_NORMAL);
			// Activating controls
			enable_RIT();
		}
	// wfi = Wait For Interrupt.
	__ASM("wfi");
	}
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
