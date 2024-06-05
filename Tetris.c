#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

// 掉落時間
#define FALL_DELAY 500
#define RENDER_DELAY 100

// 鍵盤對照表
#define LEFT_KEY 0x25
#define RIGHT_KEY 0x27
#define ROTATE_KEY 0x26
#define DOWN_KEY 0x28
#define FALL_KEY 0x20

// 判斷是否有按下按鈕的函式
#define LEFT_FUNC() GetAsyncKeyState(LEFT_KEY) & 0x8000
#define RIGHT_FUNC() GetAsyncKeyState(RIGHT_KEY) & 0x8000
#define ROTATE_FUNC() GetAsyncKeyState(ROTATE_KEY) & 0x8000
#define DOWN_FUNC() GetAsyncKeyState(DOWN_KEY) & 0x8000
#define FALL_FUNC() GetAsyncKeyState(FALL_KEY) & 0x8000

typedef enum
{
	RED = 41,
	GREEN,
	YELLOW,
	BLUE,
	PURPLE,
	CYAN,
	WHITE,
	BLACK = 0,
} Color;

typedef enum
{
	EMPTY = -1,
	I,
	J,
	L,
	O,
	S,
	T,
	Z
} ShapeId;

typedef struct
{
	ShapeId shape;
	Color color;
	int size;
	char rotates[4][4][4];
} Shape;

typedef struct
{
	int x;
	int y;
	int score;
	int rotate;
	int fallTime;
	bool gameOver;
	int high_score;
	ShapeId queue[4];
} State;

typedef struct
{
	Color color;
	ShapeId shape;
	bool current;
} Block;

Shape shapes[7] = {
	{.shape = I,
	 .color = CYAN,
	 .size = 4,
	 .rotates =
		 {
			 {{0, 0, 0, 0},
			  {1, 1, 1, 1},
			  {0, 0, 0, 0},
			  {0, 0, 0, 0}},
			 {{0, 0, 1, 0},
			  {0, 0, 1, 0},
			  {0, 0, 1, 0},
			  {0, 0, 1, 0}},
			 {{0, 0, 0, 0},
			  {0, 0, 0, 0},
			  {1, 1, 1, 1},
			  {0, 0, 0, 0}},
			 {{0, 1, 0, 0},
			  {0, 1, 0, 0},
			  {0, 1, 0, 0},
			  {0, 1, 0, 0}}}},
	{.shape = J,
	 .color = BLUE,
	 .size = 3,
	 .rotates =
		 {
			 {{1, 0, 0},
			  {1, 1, 1},
			  {0, 0, 0}},
			 {{0, 1, 1},
			  {0, 1, 0},
			  {0, 1, 0}},
			 {{0, 0, 0},
			  {1, 1, 1},
			  {0, 0, 1}},
			 {{0, 1, 0},
			  {0, 1, 0},
			  {1, 1, 0}}}},
	{.shape = L,
	 .color = YELLOW,
	 .size = 3,
	 .rotates =
		 {
			 {{0, 0, 1},
			  {1, 1, 1},
			  {0, 0, 0}},
			 {{0, 1, 0},
			  {0, 1, 0},
			  {0, 1, 1}},
			 {{0, 0, 0},
			  {1, 1, 1},
			  {1, 0, 0}},
			 {{1, 1, 0},
			  {0, 1, 0},
			  {0, 1, 0}}}},
	{.shape = O,
	 .color = WHITE,
	 .size = 2,
	 .rotates =
		 {
			 {{1, 1},
			  {1, 1}},
			 {{1, 1},
			  {1, 1}},
			 {{1, 1},
			  {1, 1}},
			 {{1, 1},
			  {1, 1}}}},
	{.shape = S,
	 .color = GREEN,
	 .size = 3,
	 .rotates =
		 {
			 {{0, 1, 1},
			  {1, 1, 0},
			  {0, 0, 0}},
			 {{0, 1, 0},
			  {0, 1, 1},
			  {0, 0, 1}},
			 {{0, 0, 0},
			  {0, 1, 1},
			  {1, 1, 0}},
			 {{1, 0, 0},
			  {1, 1, 0},
			  {0, 1, 0}}}},
	{.shape = T,
	 .color = PURPLE,
	 .size = 3,
	 .rotates =
		 {
			 {{0, 1, 0},
			  {1, 1, 1},
			  {0, 0, 0}},

			 {{0, 1, 0},
			  {0, 1, 1},
			  {0, 1, 0}},
			 {{0, 0, 0},
			  {1, 1, 1},
			  {0, 1, 0}},
			 {{0, 1, 0},
			  {1, 1, 0},
			  {0, 1, 0}}}},
	{.shape = Z,
	 .color = RED,
	 .size = 3,
	 .rotates =
		 {
			 {{1, 1, 0},
			  {0, 1, 1},
			  {0, 0, 0}},
			 {{0, 0, 1},
			  {0, 1, 1},
			  {0, 1, 0}},
			 {{0, 0, 0},
			  {1, 1, 0},
			  {0, 1, 1}},
			 {{0, 1, 0},
			  {1, 1, 0},
			  {1, 0, 0}}}},
};

void resetBlock(Block* block)
{
	block->color = BLACK;
	block->shape = EMPTY;
	block->current = false;
}

void setBlock(Block* block, Color color, ShapeId shape, bool current)
{
	block->color = color;
	block->shape = shape;
	block->current = current;
}

State* init(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH])
{
	State* statePtr = (State*)malloc(sizeof(State));

	if (statePtr != NULL)
	{
		statePtr->x = CANVAS_WIDTH / 2;
		statePtr->y = 0;
		statePtr->score = 0;
		statePtr->rotate = 0;
		statePtr->fallTime = 0;
		statePtr->high_score = 0;
		statePtr->gameOver = false;

		for (int i = 0; i < 4; i++)
		{
			statePtr->queue[i] = rand() % 7;
		}

		for (int i = 0; i < CANVAS_HEIGHT; i++)
		{
			for (int j = 0; j < CANVAS_WIDTH; j++)
			{
				resetBlock(&canvas[i][j]);
			}
		}

		Shape shapeData = shapes[statePtr->queue[0]];

		for (int i = 0; i < shapeData.size; i++)
		{
			for (int j = 0; j < shapeData.size; j++)
			{
				if (shapeData.rotates[0][i][j])
				{
					setBlock(&canvas[statePtr->y + i][statePtr->x + j], shapeData.color, statePtr->queue[0], true);
				}
			}
		}
	}

	return statePtr;
}

void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
	printf("\033[0;0H\n");
	for (int i = 0; i < CANVAS_HEIGHT; i++)
	{
		printf("|");
		for (int j = 0; j < CANVAS_WIDTH; j++)
		{
			printf("\033[%dm\u3000", canvas[i][j].color);
		}
		printf("\033[0m|\n");
	}

	// 輸出Next:
	printf("\033[%d;%dHNext:", 3, CANVAS_WIDTH * 2 + 5);
	printf("\033[%d;%dHhigh score: %d", 20, CANVAS_WIDTH * 2 + 5, state->high_score);
	printf("\033[%d;%dHscore: %d", 21, CANVAS_WIDTH * 2 + 5, state->score);
	// 輸出有甚麼方塊
	for (int i = 1; i <= 3; i++)
	{
		Shape shapeData = shapes[state->queue[i]];
		for (int j = 0; j < 4; j++)
		{
			printf("\033[%d;%dH", i * 4 + j, CANVAS_WIDTH * 2 + 15);
			for (int k = 0; k < 4; k++)
			{
				if (j < shapeData.size && k < shapeData.size && shapeData.rotates[0][j][k])
				{
					printf("\x1b[%dm  ", shapeData.color);
				}
				else
				{
					printf("\x1b[0m  ");
				}
			}
		}
	}
	return;
}

bool move(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId)
{
	Shape shapeData = shapes[shapeId];
	int size = shapeData.size;

	// 判斷方塊有沒有不符合條件
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (shapeData.rotates[newRotate][i][j])
			{
				// 判斷有沒有出去邊界
				if (newX + j < 0 || newX + j >= CANVAS_WIDTH || newY + i < 0 || newY + i >= CANVAS_HEIGHT)
				{
					return false;
				}
				// 判斷有沒有碰到別的方塊
				if (!canvas[newY + i][newX + j].current && canvas[newY + i][newX + j].shape != EMPTY)
				{
					return false;
				}
			}
		}
	}

	// 移除方塊舊的位置
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (shapeData.rotates[originalRotate][i][j])
			{
				resetBlock(&canvas[originalY + i][originalX + j]);
			}
		}
	}

	// 移動方塊至新的位置
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (shapeData.rotates[newRotate][i][j])
			{
				setBlock(&canvas[newY + i][newX + j], shapeData.color, shapeId, true);
			}
		}
	}

	return true;
}

int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH])

{
	for (int i = 0; i < CANVAS_HEIGHT; i++)
	{
		for (int j = 0; j < CANVAS_WIDTH; j++)
		{
			if (canvas[i][j].current)
			{
				canvas[i][j].current = false;
			}
		}
	}

	int linesCleared = 0;

	for (int i = CANVAS_HEIGHT - 1; i >= 0; i--)
	{
		// 原本的code
		bool isFull = true;
		for (int j = 0; j < CANVAS_WIDTH; j++)
		{
			if (canvas[i][j].shape == EMPTY)
			{
				isFull = false;
				break;
			}
		}
		// 新增的
		if (isFull)
		{
			linesCleared += 1;

			for (int j = i; j > 0; j--)
			{
				for (int k = 0; k < CANVAS_WIDTH; k++)
				{
					setBlock(&canvas[j][k], canvas[j - 1][k].color, canvas[j - 1][k].shape, false);
					resetBlock(&canvas[j - 1][k]);
				}
			}
			i++;
		}
	}
	return linesCleared;
}

int read_high_score(FILE** file)
{
	int h_score = 0;

	// 讀取檔案內的歷史最高紀錄
	*file = fopen("high_score.txt", "r");
	if (*file == NULL)
	{
		return 0;
	}
	else
	{
		if (fscanf(*file, "%d", &h_score) != 1)
		{
			h_score = 0;
		}
		fclose(*file);
	}
	return h_score;
}

void store(FILE** file, State* state)
{
	// 檔案開啟模式為 "r+"，如果檔案不存在則建立新檔案
	*file = fopen("high_score.txt", "r+");
	if (*file == NULL)
	{
		*file = fopen("high_score.txt", "w");
		if (*file == NULL)
		{
			printf("Failed to open file for writing.\n");
		}
	}

	// 如果當前分數大於歷史最高分數，則將目前的最高分寫入檔案
	if (state->score > state->high_score)
	{
		state->high_score = state->score;
		fseek(*file, 0, SEEK_SET);
		fprintf(*file, "%d", state->high_score);
	}

	fclose(*file);
}

void handleRotation(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	if (ROTATE_FUNC()) {
		int newRotate = (state->rotate + 1) % 4;
		if (move(canvas, state->x, state->y, state->rotate, state->x, state->y, newRotate, state->queue[0])) {
			state->rotate = newRotate;
		}
	}
}

void handleMovement(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	if (LEFT_FUNC()) {
		if (move(canvas, state->x, state->y, state->rotate, state->x - 1, state->y, state->rotate, state->queue[0])) {
			state->x -= 1;
		}
	}
	else if (RIGHT_FUNC()) {
		if (move(canvas, state->x, state->y, state->rotate, state->x + 1, state->y, state->rotate, state->queue[0])) {
			state->x += 1;
		}
	}
	else if (DOWN_FUNC()) {
		state->fallTime = FALL_DELAY;
	}
}

void handleFall(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	if (FALL_FUNC()) {
		while (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0])) {
			state->y++;
		}
		state->fallTime = 0;
	}
}

void updateFall(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	state->fallTime += RENDER_DELAY;

	while (state->fallTime >= FALL_DELAY) {
		state->fallTime -= FALL_DELAY;
		if (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0])) {
			state->y++;
		}
		else {
			state->score += clearLine(canvas) * 100;

			state->x = CANVAS_WIDTH / 2;
			state->y = 0;
			state->rotate = 0;
			state->queue[0] = state->queue[1];
			state->queue[1] = state->queue[2];
			state->queue[2] = state->queue[3];
			state->queue[3] = rand() % 7;

			if (!move(canvas, state->x, state->y, state->rotate, state->x, state->y, state->rotate, state->queue[0])) {
				printf("\033[%d;%dH\x1b[41m GAME OVER \x1b[0m\033[%d;%dH", CANVAS_HEIGHT - 3, CANVAS_WIDTH * 2 + 5, CANVAS_HEIGHT + 5, 0);
				state->gameOver = true;
			}
		}
	}
}

void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
	handleRotation(canvas, state);
	handleMovement(canvas, state);
	handleFall(canvas, state);
	updateFall(canvas, state);
}

void pauseGame()
{
	printf("\033[%d;%dHGame Paused. Press any key to continue...", 19, CANVAS_WIDTH * 2 + 5); // 輸出暫停訊息
	while (true)
	{ // 進入暫停迴圈
		if (_kbhit())
		{             // 再次檢查是否有鍵盤輸入
			_getch(); // 獲取鍵盤輸入
			break;    // 離開暫停迴圈
		}
	}
	// 清除暂停信息
	printf("\033[%d;%dH\033[K", 19, CANVAS_WIDTH * 2 + 5); // 使用ANSI转义码清除暂停信息所在行
}

void end_output(State* state)
{
	printf("\033[%d;%dH\x1b[41m GAME OVER \x1b[0m\033[%d;%dH", CANVAS_HEIGHT - 3, CANVAS_WIDTH * 2 + 5, CANVAS_HEIGHT + 5, 0);
	system("cls");
	// 判斷有沒有破紀錄 有就輸出恭喜訊息 沒有就輸出距離最高分數還有多少分
	printf("your score: %d\n", state->score);
	if (state->score > state->high_score)
	{
		printf("Congratulations! You broke the record!\n");
	}
	else if (state->score == 0 && state->high_score == 0)
	{
		printf("You didn't break the record, but you can do it!\n");
	}
	else if (state->score < state->high_score)
	{
		printf("Your high score: %d\n", state->high_score);
		printf("You didn't break the record, but you can do it!\n");
		printf("You are %d points away from the high score!\n", state->high_score - state->score);
	}
	else if (state->score == state->high_score)
	{
		printf("You are tied with the high score!\n");
	}

	printf("Press 'r' to restart or 'q' to quit: \n\n\n");
}

int main()
{
	srand(time(NULL));

	FILE* histry_file;
	Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
	State* state = init(canvas);
	state->high_score = read_high_score(&histry_file);

	while (true)
	{
		printCanvas(canvas, state);
		logic(canvas, state);

		if (_kbhit())
		{                          // 檢查是否有鍵盤輸入
			char input = _getch(); // 獲取鍵盤輸入
			if (input == 27)
			{                // 如果按下的是 ESC 鍵
				pauseGame(); // 呼叫暫停函數
			}
		}
		if (state->gameOver)
		{
			end_output(state);
			store(&histry_file, state);

			char input;
			do
			{
				input = _getch();
			} while (input != 'r' && input != 'q');

			if (input == 'r')
			{
				system("cls");
				free(state);
				state = init(canvas);
				state->high_score = read_high_score(&histry_file);

				continue;
			}
			else if (input == 'q')
			{
				system("cls");
				break;
			}
		}
		Sleep(RENDER_DELAY); // 控制游戏速度
	}
	free(state);

	return 0;
}
