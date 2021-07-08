#pragma once
#include "Board.h"

constexpr unsigned long long center_16_bitmask = 66229406269440ULL;
constexpr unsigned long long center_4_bitmask = 103481868288ULL;
constexpr unsigned long long corners = 0x8100000000000081;
constexpr unsigned long long C_squares = 0x42C300000000C342;




inline const int evaluate(const Board& b)
{
	const bitboard black_bb = b.get_board()[COLOR_BLACK];
	const bitboard white_bb = b.get_board()[COLOR_WHITE];

	//black side to move perspective
	int total_score = b.getScore();

	//center 16 score
	int center_16_score = (__popcnt64(black_bb & center_16_bitmask) - __popcnt64(white_bb & center_16_bitmask)) * 3;

	// center 4 score 
	int center_4_score = (__popcnt64(black_bb & center_4_bitmask) - __popcnt64(white_bb & center_4_bitmask)) * 3;

	//corners score
	int corner_score = (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * 12;

	//C-squares punishment (smaller than corners, so that taking corners is always beneficial, but not C squares alone)
	int C_squares_punishment = (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * -2;

	total_score += center_16_score + center_4_score + corner_score + C_squares_punishment;

	//side to move perspective
	return total_score * ((b.getSideToMove() == COLOR_BLACK) ? 1 : -1);
}
