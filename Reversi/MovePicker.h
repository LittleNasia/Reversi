#pragma once 
#include "Board.h"
#include "TT.h"
#include "Search.h"

struct weighted_move
{
	int8_t move;
	int16_t weight;
};

class MovePicker
{
public:
	MovePicker(Board& b, const TT_entry& entry, const bool found_tt_entry, const search::SearchInfo& s);

	uint16_t get_move();

	constexpr int get_move_count() const { return move_count; }
private:
	int move_count;
	int current_move = 0;
	weighted_move weighted_moves[32];
};

