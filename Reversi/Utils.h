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

