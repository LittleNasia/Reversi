#pragma once
#include "board.h"
#include "mcts_node_allocator.h"

namespace MCTS
{
	inline constexpr int total_simulations = 60;
	int search_move(board& b, int& score);
}