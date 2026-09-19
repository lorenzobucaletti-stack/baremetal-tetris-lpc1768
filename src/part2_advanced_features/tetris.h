#ifndef _TETRIS_H_
#define _TETRIS_H_

// Include
#include "LPC17xx.h"
#include <stdint.h>

// Define
// Logic dimensions of the grif
#define Board_ROWS 20
#define Board_COLS 10
// Pieces identifiers
#define EMPTY 0
#define I_SHAPE 1
#define J_SHAPE 2
#define L_SHAPE 3
#define O_SHAPE 4
#define S_SHAPE 5
#define T_SHAPE 6
#define Z_SHAPE 7
// Graphic dimensions
#define BLOCK_SIZE 16 // every block is 12*12
#define BOARD_HEIGHT_PX (Board_ROWS * BLOCK_SIZE) // 20 * 12 = 240 pixels
#define BOARD_WIDTH_PX (Board_COLS * BLOCK_SIZE) // 10 * 12 = 120 pixels
// Screen position
#define BOARD_X 0
#define BOARD_Y 0

// Struct for the informantions on the piece position and orentation
typedef struct{
	int x;           // current column (0-9)
	int y;           // current row (0-19)
	int typeID;      // type of shape ID
	int shape[4][4]; // current shape (also rotated)
} CurrentPiece;
extern CurrentPiece currentPiece;
// Declare the matrix representing the board
extern int Board[Board_ROWS][Board_COLS];
// Declare the array for the tetrominos' colors
extern const uint16_t TETROMINO_COLORS[8];

// Declaration of tetris functions
void Init_Tetris(void);
int Tetris_SpawnPiece(void);
int Tetris_HasLost(void);
void PrintStats(void);
void DrawPiece(uint16_t color);
void DrawCurrentPiece(void);
void Tetris_Update(void);
void Tetris_Move(int direction);
void Tetris_Rotate(void);
void Tetris_HardDrop(void);
int CheckCollision(int newX, int newY);
void FreezePiece(void);
void CheckLines(void);
void Tetris_UpdateHighScore(void);
void Tetris_SetStarted(int state);
int Tetris_IsStarted(void);
void Tetris_SetPause(int state);
int Tetris_IsPaused(void);

#endif