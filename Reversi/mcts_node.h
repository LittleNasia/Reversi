#pragma once
#include "board.h"

namespace MCTS
{
	constexpr int num_playthroughs = 120;
	struct mcts_node
	{
		int move;
		int playouts;
		float wins;
		mcts_node* parent = nullptr;
		//assumes no more than 32 children per pos
		mcts_node* children[32];
		int num_children = 0;

		constexpr inline const bool is_root() const { return parent == nullptr; }
		constexpr inline const bool is_leaf() const { return !num_children; }

		void evaluate_children(board& node_pos);
		void populate_children(board& node_pos);
		

	};
}