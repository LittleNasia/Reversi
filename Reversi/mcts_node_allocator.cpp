#include "mcts_node_allocator.h"

namespace MCTS
{
	namespace mcts_node_allocator
	{
		static mcts_node buff[num_nodes];
		inline static int num_allocated_nodes = 0;

		mcts_node* alloc_new()
		{
			if (num_allocated_nodes >= num_nodes)
			{
				return nullptr;
			}
			return &(buff[num_allocated_nodes++]);
		}

		void reset()
		{
			num_allocated_nodes = 0;
			buff[num_allocated_nodes].num_children = 0;
		}
	}
}
