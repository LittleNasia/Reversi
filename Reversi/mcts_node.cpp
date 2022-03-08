#include "mcts_node_allocator.h"
#include "search.h"

namespace MCTS
{
	void mcts_node::evaluate_children(board& node_pos)
	{
		for (int child = 0; child < num_children; child++)
		{
			node_pos.do_move(children[child]->move);
			for (int game = 0; game < num_playthroughs; game++)
			{
				// we keep track of how many extra moves were made 
				// it's to make sure the node_pos is left 
				// exactly as it was before calling this function
				int num_moves_to_undo = 0;
				// simulate the game with random moves
				while (!node_pos.is_over())
				{
					node_pos.do_random_move();
					num_moves_to_undo++;
				}
				// check the game result
				int score = node_pos.get_score();
				float result = 0.5;
				if (score)
				{
					// positive score means black wins
					// negative core means white wins
					result = (score > 0) ? 1 : 0;	
				}
				// undo all the moves made in simulation
				for (int move_undo = 0; move_undo < num_moves_to_undo; move_undo++)
				{
					node_pos.undo_move();
				}

				// update children's score
				children[child]->playouts += 1;
				children[child]->wins += result;
			}
			node_pos.undo_move();
		}
	}

	void mcts_node::populate_children(board& b)
	{
		bitboard moves = b.get_moves_bitmask();

		while (moves)
		{
			int move = pop_bit(moves);

			//create child
			auto& new_child = children[num_children++];
			new_child = mcts_node_allocator::alloc_new();

			//set properties
			new_child->parent = this;
			new_child->move = move;
			new_child->playouts = 0;
			new_child->wins = 0;
			new_child->num_children = 0;
		}

		evaluate_children(b);
	}
}