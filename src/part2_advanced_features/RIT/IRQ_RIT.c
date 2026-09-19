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
#include "adc.h"
#include "music.h"

#define SPEED_NORMAL 0x017D7840 // Normal speed of the pieces
// Peripheral clock = 25 MHz --> 1 second = 25.000.000 CCs = 0x017D7840, this should be used on the real board
# define SPEED_SOFTDROP 0X00BEBC20 // Double speed
#define SPEED_SLOW 0x02FAF080   // Low speed

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
extern volatile uint32_t current_base_interval; // Value computed by th ADC
volatile int slow_down_timer = 0; // Slow down counter
void ActivatePowerupS(void) {
    slow_down_timer = 300;
}

// State variables for sound effects
static int sfx_playing = 0;      // 0 = No, 1 = Yes
static int sfx_current_note = 0; // Current note of the SFX index
static int sfx_type = 0;         // Which SFX is playing

// SFX DEFINITIONS
// Powerup SFX
NOTE sfx_powerup_seq[] = {
  {c4, time_semicroma}, {e4, time_semicroma}, {g4, time_semicroma}, {c5, time_semiminima}
};

// Malus SFX
NOTE sfx_malus_seq[] = {
  {f4, time_semicroma}, {b3, time_semicroma}, {g3, time_semiminima}
};

// Game Over SFX
NOTE sfx_gameover_seq[] = {
  {b3, time_semicroma},
  {f4, time_semicroma},
  {pause, time_semicroma},
  {f4, time_semicroma},
  {f4, time_semicroma},
  {e4, time_semicroma},
  {d4, time_semicroma},
  {c4, time_semicroma},
	{g3, time_semicroma},
	{e3, time_semicroma},
	{c3, time_semiminima}
};
// Function for activating the SFX
void PlaySoundEffect(int id) {
    sfx_type = id;
    sfx_current_note = 0;
    sfx_playing = 1; 
    reset_timer(2);  
}

/* --- TETRIS THEME (Korobeiniki) --- */
// Note: pitch, duration
NOTE song[] = {
    {e4, time_semiminima}, {b3, time_croma}, {c4, time_croma}, {d4, time_semiminima}, {c4, time_croma}, {b3, time_croma},
    {a3, time_semiminima}, {a3, time_croma}, {c4, time_croma}, {e4, time_semiminima}, {d4, time_croma}, {c4, time_croma},
    {b3, time_semiminima}, {b3, time_croma}, {c4, time_croma}, {d4, time_semiminima}, {e4, time_semiminima},
    {c4, time_semiminima}, {a3, time_semiminima}, {a3, time_semiminima}, {pause, time_semiminima},
    
    {d4, time_semiminima}, {d4, time_croma}, {f4, time_croma}, {a4, time_semiminima}, {g4, time_croma}, {f4, time_croma},
    {e4, time_semiminima}, {e4, time_croma}, {c4, time_croma}, {e4, time_semiminima}, {d4, time_croma}, {c4, time_croma},
    {b3, time_semiminima}, {b3, time_croma}, {c4, time_croma}, {d4, time_semiminima}, {e4, time_semiminima},
    {c4, time_semiminima}, {a3, time_semiminima}, {a3, time_semiminima}, {pause, time_semiminima}
};
// Current note index
static int currentNote = 0;

void RIT_IRQHandler (void)
{					
	// Starts ADC conversion (Polling every 50ms)
  ADC_start_conversion();
	
	// Timer Power-up S management (Slow Down)
  if (slow_down_timer > 0) {
    slow_down_timer--; // Decrement every 50ms
  }

  // Compute the current target speed
  uint32_t target_speed;
  
  if (slow_down_timer > 0) {
    // If the slow down powerup is on, ignore the potentiometer
    target_speed = SPEED_SLOW; 
  } else {
    // Otherwise I use it
    target_speed = current_base_interval; 
  }
	
	// SOUNDTRACK MANAGEMENT
  // First, checking if I'm playing something
  if(!isNotePlaying()) {
    // Priority to managing the SFX
    if (sfx_playing) {
      NOTE *current_sfx_array;
      int sfx_len = 0;
			
      // Select the right array (melody)
      switch(sfx_type) {
        case SFX_POWERUP: 
          current_sfx_array = sfx_powerup_seq; 
          sfx_len = sizeof(sfx_powerup_seq)/sizeof(sfx_powerup_seq[0]);
          break;
        case SFX_MALUS: 
          current_sfx_array = sfx_malus_seq; 
          sfx_len = sizeof(sfx_malus_seq)/sizeof(sfx_malus_seq[0]);
          break;
        case SFX_GAMEOVER: 
          current_sfx_array = sfx_gameover_seq; 
          sfx_len = sizeof(sfx_gameover_seq)/sizeof(sfx_gameover_seq[0]);
          break;
        default: sfx_len = 0; break;
      }
			
      // Play the current not of the SFX
      if (sfx_current_note < sfx_len) {
        playNote(current_sfx_array[sfx_current_note]);
        sfx_current_note++;
      } else {
        // SFX finished
        sfx_playing = 0; // Back to soundtrack
      }
    }
    // Soundtrack management
    else if(Tetris_IsStarted() && !Tetris_IsPaused() && !Tetris_HasLost()) {
      playNote(song[currentNote]);
      currentNote++;
			// If the melody has finished, restart it
      if(currentNote >= (sizeof(song) / sizeof(song[0]))) {
        currentNote = 0;
      }
    }
}
	
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
				// Music reset
        currentNote = 0;
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
          DrawCurrentPiece(); // Redraw piece
				} else {
					// Set pause mode
					Tetris_SetPause(1);
					// Switch off the timer
					disable_timer(0);
					// Stops the music
					reset_timer(2);
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

	
	// Management of the dynamic SOFT DROP (Joystick DOWN)
  // "Holding joystick down doubles the current falling speed"
	/* ---------------- JOISTICK DOWN (P1.26) ----------------- */
	// SOFT DROP
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
		uint32_t fast_speed = target_speed/ 2;
		// We set MR0 to the value of the ADC divided by 2
    LPC_TIM0->MR0 = fast_speed;
		if(LPC_TIM0->TC >= fast_speed) {
			Tetris_Update();
			LPC_TIM0->TC = 0;
    }
	}
	else{
		// Reset the normal speed imposed by the potentiometer
    LPC_TIM0->MR0 = target_speed;
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
	
	/* ---------------- LIVE SPEED UPDATE ----------------- */
  if (J_Down_Pressed == 0 && Tetris_IsStarted() && !Tetris_IsPaused()) {
    // Update the timer speed with the last value read by the potentiometer
    LPC_TIM0->MR0 = target_speed;
		if (LPC_TIM0->TC >= target_speed) {
          LPC_TIM0->TC = 0;                   
      }
  }

	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
