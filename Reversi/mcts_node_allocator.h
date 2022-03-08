#pragma once
#include "mcts_node.h"


namespace MCTS
{
	namespace mcts_node_allocator
	{
		inline static constexpr int num_nodes = 2 << 15;
		mcts_node* alloc_new();
		void reset();
	}
};

