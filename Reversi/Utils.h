#pragma once
#include "BitUtilities.h"
#include <Eigen/Core>
enum Color
{
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_NONE
};

struct Point
{
	int16_t row;
	int16_t col;
};

using ScalarType = float;
inline constexpr int NN_input_channels = 2;

enum Direction
{
	DIRECTION_UP_LEFT,
	DIRECTION_UP,
	DIRECTION_UP_RIGHT,
	DIRECTION_RIGHT,
	DIRECTION_DOWN_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_DOWN_LEFT,
	DIRECTION_LEFT,
	DIRECTION_NONE
};

inline constexpr Point directions[DIRECTION_NONE] =
{
	{-1,-1},
	{-1,0},
	{-1,1},
	{0,1},
	{1,1},
	{1,0},
	{1,-1},
	{0,-1}
};

//for each square on the board, keeps track how many iterations per loop for each direction are to be done
inline constexpr int capture_iteration_count[64][DIRECTION_NONE] =
{
{7,7,0,0,0,0,0,7},
{6,7,1,1,0,0,0,6},
{5,7,2,2,0,0,0,5},
{4,7,3,3,0,0,0,4},
{3,7,4,4,0,0,0,3},
{2,7,5,5,0,0,0,2},
{1,7,6,6,0,0,0,1},
{0,7,7,7,0,0,0,0},
{6,6,0,0,0,1,1,7},
{6,6,1,1,1,1,1,6},
{5,6,2,2,1,1,1,5},
{4,6,3,3,1,1,1,4},
{3,6,4,4,1,1,1,3},
{2,6,5,5,1,1,1,2},
{1,6,6,6,1,1,1,1},
{0,6,6,7,1,1,0,0},
{5,5,0,0,0,2,2,7},
{5,5,1,1,1,2,2,6},
{5,5,2,2,2,2,2,5},
{4,5,3,3,2,2,2,4},
{3,5,4,4,2,2,2,3},
{2,5,5,5,2,2,2,2},
{1,5,5,6,2,2,1,1},
{0,5,5,7,2,2,0,0},
{4,4,0,0,0,3,3,7},
{4,4,1,1,1,3,3,6},
{4,4,2,2,2,3,3,5},
{4,4,3,3,3,3,3,4},
{3,4,4,4,3,3,3,3},
{2,4,4,5,3,3,2,2},
{1,4,4,6,3,3,1,1},
{0,4,4,7,3,3,0,0},
{3,3,0,0,0,4,4,7},
{3,3,1,1,1,4,4,6},
{3,3,2,2,2,4,4,5},
{3,3,3,3,3,4,4,4},
{3,3,3,4,4,4,3,3},
{2,3,3,5,4,4,2,2},
{1,3,3,6,4,4,1,1},
{0,3,3,7,4,4,0,0},
{2,2,0,0,0,5,5,7},
{2,2,1,1,1,5,5,6},
{2,2,2,2,2,5,5,5},
{2,2,2,3,3,5,4,4},
{2,2,2,4,4,5,3,3},
{2,2,2,5,5,5,2,2},
{1,2,2,6,5,5,1,1},
{0,2,2,7,5,5,0,0},
{1,1,0,0,0,6,6,7},
{1,1,1,1,1,6,6,6},
{1,1,1,2,2,6,5,5},
{1,1,1,3,3,6,4,4},
{1,1,1,4,4,6,3,3},
{1,1,1,5,5,6,2,2},
{1,1,1,6,6,6,1,1},
{0,1,1,7,6,6,0,0},
{0,0,0,0,0,7,7,7},
{0,0,0,1,1,7,6,6},
{0,0,0,2,2,7,5,5},
{0,0,0,3,3,7,4,4},
{0,0,0,4,4,7,3,3},
{0,0,0,5,5,7,2,2},
{0,0,0,6,6,7,1,1},
{0,0,0,7,7,7,0,0}
};

inline static unsigned long x = __rdtsc(), y = 362436069, z = 521288629;

inline constexpr unsigned long rng() 
{
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;

	unsigned long t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;

	return z;
}