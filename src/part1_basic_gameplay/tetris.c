// Includes
#include <stdint.h>
#include "tetris.h"
#include "GLCD.h"
#include <stdlib.h> // needed for rand()
#include <stdio.h> // needed for sprintf
#include "timer.h"
#include "RIT.h"


// Global variables
int Board[Board_ROWS][Board_COLS]; // Board occupied/free blocks
CurrentPiece currentPiece; // Global variable for the piece status
int score = 0; // Global variable for the score
int lines = 0; // Global variable for the completed lines
static int internal_game_over = 0; // Flag for game over
static int highScore = 0; // 0 only when starting a game
static int game_state_started = 0; // 0 = reset status, 1 = game started
static int game_state_paused = 1; // 0 = game not paused, 1 = game paused 

// Define tetrominoes for every type of shape
uint16_t TETROMINOES[7][4][4] = {
    // 1. I-Shape
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 2. J-Shape
    {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 3. L-Shape
    {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 4. O-Shape
    {
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 5. S-Shape
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 6. T-Shape
    {
        {1, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // 7. Z-Shape
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }
};

// Define orange color
#ifndef Orange
#define Orange 0xFD20
#endif
// Define tetrominoes' colors
const uint16_t TETROMINO_COLORS[8] = {
    Black,   // 0: Empty
    Cyan,    // 1: I-Shape
    Blue,    // 2: J-Shape
    Orange,  // 3: L-Shape
    Yellow,  // 4: O-Shape
    Green,   // 5: S-Shape
    Magenta, // 6: T-Shape
    Red      // 7: Z-Shape
};


// Function for initializing the display graphic and stats
void Init_Tetris(void) {
  int i, j;
	
	// Deleting previous match stats
	score = 0;
	lines = 0;
	internal_game_over = 0;
	// Reset game/pause states
	game_state_started = 0;
	game_state_paused = 1;
	
	// Initializing the Board matrix to all EMPTY
  for(i = 0; i < Board_ROWS; i++) {
      for(j = 0; j < Board_COLS; j++) {
          Board[i][j] = EMPTY;
      }
  }
  
  // Clear the screen with black background
  LCD_Clear(Black);
  // Draw the grid
  for(i = 0; i <= Board_COLS; i++) {
		int x = i * BLOCK_SIZE; // x-coordinate
      LCD_DrawLine(BOARD_X + x, BOARD_Y, BOARD_X + x, BOARD_Y + BOARD_HEIGHT_PX, Grey);
  }
  for(i = 0; i <= Board_ROWS; i++) {
		int y = i * BLOCK_SIZE; // y-coordinate
		if(i *BLOCK_SIZE >= 320){
			y = 319;
		}
      LCD_DrawLine(BOARD_X, BOARD_Y + y, BOARD_X + BOARD_WIDTH_PX, BOARD_Y + y, Grey);
  }

  // Draw text interface
  // Title
  GUI_Text(175, 20, (uint8_t *)"TETRIS", Red, Black);
   // Score
  GUI_Text(165, 60, (uint8_t *)"Score:", White, Black);
  GUI_Text(165, 80, (uint8_t *)"0", White, Black);
  // Lines
  GUI_Text(165, 105, (uint8_t *)"Lines:", White, Black);
  GUI_Text(165, 125, (uint8_t *)"0", White, Black);
	// Highscore
	char highScore_text[20];
	GUI_Text(165, 150, (uint8_t *)"HS:", Yellow, Black);
	sprintf(highScore_text, "%d", highScore);
	GUI_Text(165, 175, (uint8_t *)highScore_text, Yellow, Black);
	// Start interface
	GUI_Text(170, 240, (uint8_t *)" Press", White, Black);
	GUI_Text(167, 260, (uint8_t *)"  KEY1 ", White, Black);
  GUI_Text(167, 280, (uint8_t *)"to START", White, Black);
}

// Function for defining the spawning piece shape and initial position
// It also controls if there is a game over or not
int Tetris_SpawnPiece(void){
	int i, j;
	
	// Variable to choose a random shape
	int rand_shape = rand() % 7;
	// Save the ID of the shape of the piece
	currentPiece.typeID = rand_shape + 1;
	
	// Spawn position of the piece (center)
	currentPiece.x = 3;
	currentPiece.y = 0;
	
	// Copy of the occupied blocks
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			currentPiece.shape[i][j] = TETROMINOES[rand_shape][i][j];
		}
	}
	
	// Check for a game over (there is an instant collision)
	if(CheckCollision(currentPiece.x, currentPiece.y) != 0){
		disable_timer(0); // Stops pieces from falling
		internal_game_over = 1; // Game over flag is set
		return 1;
	}
	return 0;
}

// Return the game over flag
int Tetris_HasLost(void) {
	return internal_game_over;
}

// Print final stats of the game
void PrintStats(void){
	LCD_Clear(Black);
	// Losing message
	GUI_Text(84, 100, (uint8_t*)"GAME OVER", Red, Black);
	// Final stats
	char final_score[30];
	sprintf(final_score, "Final score: %d", score);
	GUI_Text(60, 130, (uint8_t*)final_score, White, Black);
	// Waiting for letting analyze the stats
	volatile int k;
	for(k = 0; k < 1000000; k++);
}

// Function for drawing blocks of a given color
void DrawPiece(uint16_t color){
	int i, j, k;
	
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			// "if the cell of the shape contains 1"
			if(currentPiece.shape[i][j] == 1){
				// compute the exact pixel from which starting drawing the block
				int px = BOARD_X + (currentPiece.x + j)*BLOCK_SIZE;
				int py = BOARD_Y + (currentPiece.y + i)*BLOCK_SIZE;
				
				// Draw the entire block
				for(k = 1; k < BLOCK_SIZE - 1; k++){
					// Draw an horizontal line from left to right
					LCD_DrawLine(px + 1, py + k, px + BLOCK_SIZE - 2, py + k, color);
					/* I start from k = 1 (and not 0) and arrive to BLOCK_SIZE - 1, in order
					to prevent the grid from disappearing while the piece is falling. */
				}
			}
		}
	}
}

// Function for drawing given pieces
void DrawCurrentPiece(void) {
    // Questa riga qui funziona perché siamo dentro tetris.c!
    DrawPiece(TETROMINO_COLORS[currentPiece.typeID]);
}

// Function that updates the piece position
// It is used in the TIMER0_IRQHandler function
void Tetris_Update(void){
	// Check if there are collisions before updating the piece position ("making it fall")
	if(CheckCollision(currentPiece.x, currentPiece.y + 1) == 0){
		// Collision will not occur --> the piece keeps falling
		
		// "Erase" (draw as the color of the background) the current piece
		DrawPiece(Black);
		// Increment the y-coordinate
		currentPiece.y++;
		// Draw the piece in the new position
		DrawPiece(TETROMINO_COLORS[currentPiece.typeID]);
		
	} else {
		// Collision will occur --> the piece stops and a new one is generated
		
		FreezePiece(); // The piece stops and becomes part of the board
	}
}

// Function that checks if there are collisions for the given piece
int CheckCollision(int newX, int newY){
	int i, j;
	
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			// I check only the coloured blocks of the piece
			if(currentPiece.shape[i][j] != 0){
				int x = newX + j;
				int y = newY + i;
				
				// I check collisions with lateral walls and with the base
				if(x < 0 || x >= Board_COLS || y >= Board_ROWS){
					return 1;
				}
				// I check collisions with other blocks
				if (y >= 0 && Board[y][x] != 0){
					return 1;
				}
			}
		}
	}
	return 0;
}

// Function that freezes a piece if a collision occurs
void FreezePiece(void){
	int i, j;
	
	// Copy the current piece blocks on the matrix Board
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(currentPiece.shape[i][j] != 0){
				int x = currentPiece.x + j;
				int y = currentPiece.y + i;
				// Directly save the piece ID in the matrix Board
				if(x >= 0 && x < Board_COLS && y >= 0 && y < Board_ROWS){
					Board[y][x] = currentPiece.typeID;
				}
			}
		}
	}
	
	score += 10; // 10 points for placing a block
	
	// Update score
	char score_text[20];
	sprintf(score_text, "%d", score);
	GUI_Text(165, 80, (uint8_t *)score_text, White, Black);
	
	// Check for and, possibly, clear completed lines
  CheckLines();
	
	// Generate the next piece if Tetris_SpawnPiece() returns 0 (there are non collisions by spawning a new piece)
	if(Tetris_SpawnPiece() == 0){
		DrawCurrentPiece();
	}
}

// Function for implementing the traslation of pieces
void Tetris_Move(int direction) {
    // direction can be +1 or -1
    int newX = currentPiece.x + direction;
    int newY = currentPiece.y; // y-coordinate doesn't change
    
    // Chieck for collisions
    if (CheckCollision(newX, newY) == 0) {
        // There is no collision --> move the piece
        DrawPiece(Black);
        currentPiece.x += direction;
        DrawPiece(TETROMINO_COLORS[currentPiece.typeID]);
    }
    // There is collision --> I do nothing
}

// Function for implementing the rotation pieces
void Tetris_Rotate(void) {
	
  uint16_t shape_backup[4][4];
  int i, j;
    
  // Erase the current piece
  DrawPiece(Black);
  // Create a backup of the current piece shape
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			shape_backup[i][j] = currentPiece.shape[i][j];
		}
	}
    
  // Apply rotation
  // currentPiece.shape[j][3 - i] = shape_backup[i][j];
  for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			currentPiece.shape[j][3 - i] = shape_backup[i][j];
		}
	}
  // Check for collisions
  if (CheckCollision(currentPiece.x, currentPiece.y) != 0) {
		// There is a collision --> restore the original piece shape
		for(i = 0; i < 4; i++){
			for(j = 0; j < 4; j++){
				currentPiece.shape[i][j] = shape_backup[i][j];
			}
		}
	}
    
  // Draw the piece
	DrawPiece(TETROMINO_COLORS[currentPiece.typeID]);
}

// Function implementing the Hard Drope tecnique
void Tetris_HardDrop(void){
	// Delete the current piece
	DrawPiece(Black);
	// Check if there are any collisions: if not the piece keeps going down
	while(CheckCollision(currentPiece.x, currentPiece.y + 1) == 0) {
		currentPiece.y++;
	}
	
	DrawPiece(TETROMINO_COLORS[currentPiece.typeID]);
	FreezePiece();
}

// Function that checks if lines are completed or not
void CheckLines(void) {
  int i, j, k;
	int lines_cleared_now = 0; // counter for lines completed with a single move
  
  // Check all the lines starting from the top
  for(i = Board_ROWS - 1; i >= 0; i--) {
		
    int filled_blocks = 0;
    // Count the blocks in the i-th row
    for(j = 0; j < Board_COLS; j++) {
        if(Board[i][j] != EMPTY) {
            filled_blocks++;
        }
    }
    
    // Check if the line is full
    if (filled_blocks == Board_COLS) {
      // Shift down of the rows
      for(k = i; k > 0; k--) {
          for(j = 0; j < Board_COLS; j++) {
              Board[k][j] = Board[k-1][j];
          }
      }
      // Clear the firs row
      for(j = 0; j < Board_COLS; j++) {
          Board[0][j] = EMPTY;
      }
      // Update the lines completed
      lines_cleared_now++;
			lines++;
      // Need to check also the new line with the new configuration
      i++;
    }
		
  }
	
	// Update the score
	if(lines_cleared_now > 0){
		
		if(lines_cleared_now == 4){
			score += 600;
		} else {
			score += (lines_cleared_now * 100);
		}
		
		// Redraw the entire display based on the new Board
		// Delete the current grid and blocks and draw the remaining ones
		LCD_Clear(Black); 
		// Draw the grid
		int grid_i;
		for(grid_i = 0; grid_i <= Board_COLS; grid_i++) {
			int x = grid_i * BLOCK_SIZE; // x-coordinate
			LCD_DrawLine(BOARD_X + x, BOARD_Y, BOARD_X + x, BOARD_Y + BOARD_HEIGHT_PX, Grey);
		}
		for(grid_i = 0; grid_i <= Board_ROWS; grid_i++) {
			int y = grid_i * BLOCK_SIZE; // y-coordinate
			if(grid_i *BLOCK_SIZE >= 320){
				y = 319;
			}
		LCD_DrawLine(BOARD_X, BOARD_Y + y, BOARD_X + BOARD_WIDTH_PX, BOARD_Y + y, Grey);
		}
		
		// Draw the remaining blocks in Board
		int r, c;
		for(r = 0; r < Board_ROWS; r++) {
			for(c = 0; c < Board_COLS; c++) {
				// In Board[i][j] the pieceID is saved, useful for the color of the block to recreate
				int pieceID = Board[r][c];
				// If there is a block in Board
				if(pieceID != EMPTY) {
					// Compute the pixel position
					int px = BOARD_X + c * BLOCK_SIZE;
					int py = BOARD_Y + r * BLOCK_SIZE;
					// Draw the block
					uint16_t color = TETROMINO_COLORS[pieceID];
					int line_k;
					for(line_k = 1; line_k < BLOCK_SIZE - 1; line_k++) {
						LCD_DrawLine(px + 1, py + line_k, px + BLOCK_SIZE - 2, py + line_k, color);
					}
				}
			}
		}
		
		// Draw the stats and update score and lines
		GUI_Text(165, 20, (uint8_t *)"TETRIS", Red, Black);
		GUI_Text(165, 60, (uint8_t *)"Score:", White, Black);
		// Score
		char score_text[10];
		sprintf(score_text, "%d", score);
		GUI_Text(165, 80, (uint8_t *)score_text, White, Black);
		// Lines
		GUI_Text(165, 105, (uint8_t *)"Lines:", White, Black);
		char lines_text[10];
		sprintf(lines_text, "%d", lines);
		GUI_Text(165, 125, (uint8_t *)lines_text, White, Black);
		// High score
		char highScore_text[20];
		GUI_Text(165, 150, (uint8_t *)"HS:", Yellow, Black);
		sprintf(highScore_text, "%d", highScore);
		GUI_Text(165, 170, (uint8_t *)highScore_text, Yellow, Black);
	}
}

// Function for updating the high score
void Tetris_UpdateHighScore(void) {
    if (score > highScore) {
        highScore = score;
    }
}

// Functions for game/pause state management
void Tetris_SetStarted(int state) {
    game_state_started = state;
}

int Tetris_IsStarted(void) {
    return game_state_started;
}

void Tetris_SetPause(int state) {
    game_state_paused = state;
}

int Tetris_IsPaused(void) {
    return game_state_paused;
}
