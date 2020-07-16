#pragma once

#include <stdint.h>

#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Kilobytes(Value) (1024LL*(Value))
#define Megabytes(Value) (1024LL*Kilobytes(Value))
#define Gigabytes(Value) (1024LL*Megabytes(Value))

struct button
{
	bool EndedDown;
	uint32_t HalfTransitionCount;
};
struct game_input
{
	float dt;
	int MouseX, MouseY;
	float MouseDeltaX, MouseDeltaY;
	button MouseLeft, MouseRight;

	union
	{
		struct
		{
			button W;
			button S;
			button D;
			button A;
		};
		button Buttons[4];
	};
};

static bool
WasDown(button* Button)
{
	bool Result = (Button->HalfTransitionCount > 1) ||
		((Button->HalfTransitionCount == 1) && Button->EndedDown);

	return(Result);
}

struct game_memory
{
	uint64_t PermanentStorageSize;
	void* PermanentStorage;

	uint64_t TemporaryStorageSize;
	void* TemporaryStorage;
};

void UpdateAndRender(game_memory* Memory, game_input* Input, uint32_t BufferWidth, uint32_t BufferHeight);