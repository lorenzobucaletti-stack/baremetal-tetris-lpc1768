#include "music.h"
#include "timer.h"
#include "LPC17xx.h"

void playNote(NOTE note)
{
	if(note.freq != pause)
	{
		reset_timer(2); // Timer_2 = frequncy
		init_timer(2, note.freq);
		enable_timer(2);
	}
	reset_timer(3); // Timer_3 = duration
	init_timer(3, note.duration);
	enable_timer(3);
}

BOOL isNotePlaying(void)
{
	// Checks if Timer_2 or Timer_3 are on
	return ((LPC_TIM2->TCR != 0) || (LPC_TIM3->TCR != 0));
}
