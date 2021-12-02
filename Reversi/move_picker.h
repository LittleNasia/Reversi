#pragma once 
#include "board.h"
#include "tt.h"
#include "Search.h"

struct weighted_move
{
	int8_t move;
	int16_t weight;
};

class move_picker
{
public:
	move_picker(board& b, const tt_entry& entry, const bool found_tt_entry, const search::search_info& s);
	move_picker(board& b);

	uint16_t get_move();

	constexpr int get_move_count() const { return move_count; }
private:
	int move_count;
	int current_move = 0;
	weighted_move weighted_moves[32];
};

