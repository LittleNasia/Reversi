#include "mcts.h"

namespace MCTS
{
	// sets the root_pos to be the node_pos of the best leaf
	mcts_node* selection(board& root_pos, mcts_node* root_node)
	{
		board& curr_pos = root_pos;
		mcts_node* curr_node = root_node;
		while (!curr_node->is_leaf())
		{
			float best_score = -INFINITY;
			mcts_node* best_node = nullptr;

			for (int child = 0; child < curr_node->num_children; child++)
			{
				auto& curr_child = curr_node->children[child];
				float wins = curr_child->wins;
				// current side_to_move perspective
				if (curr_pos.get_side_to_move() == COLOR_WHITE)
				{
					wins = curr_child->playouts - wins;
				}
				float score = wins / curr_child->playouts;
				score *= std::sqrt(2 * std::log((float)curr_child->parent->playouts) / curr_child->playouts);
				if (score > best_score)
				{
					best_node = curr_child;
					best_score = score;
				}
			}
			curr_node = best_node;
			curr_pos.do_move(curr_node->move);
			//std::cout << best_score << "\n";
			//curr_pos.print_board();
		}

		return curr_node;
	}

	// brings back the node_pos back to the root_pos
	void backpropagation(mcts_node* node, board& node_pos)
	{
		int playouts = 0;
		float wins = 0;
		// get the node score by summing its children scores
		for (int child = 0; child < node->num_children; child++)
		{
			playouts += node->children[child]->playouts;
			wins += node->children[child]->wins;
		}
		node->wins += wins;
		node->playouts += playouts;
		// backpropagate the new result back to the root
		while (!node->is_root())
		{
			node_pos.undo_move();
			node = node->parent;
			node->wins += wins;
			node->playouts += playouts;
		}
	}

	int search_move(board& b, int& score)
	{
		mcts_node_allocator::reset();
		mcts_node* root_node = mcts_node_allocator::alloc_new();
		root_node->populate_children(b);
		// update root_node_score
		for (int child = 0; child < root_node->num_children; child++)
		{
			root_node->playouts += root_node->children[child]->playouts;
			root_node->wins += root_node->children[child]->wins;
		}


		for (int i = 0; i < total_simulations; i++)
		{
			//b.print_board();
			//std::cout << &root_node << "\n";
			// select the best variation 
			// "b", aka the root_pos is now the leaf node pos
			auto best_leaf = selection(b, root_node);
			//b.print_board();
			//std::cout << "selection\n";
			// expand the child
			// b is *still* the leaf node pos
			best_leaf->populate_children(b);
			//std::cout << "num children " <<  best_leaf->num_children << "\n";
			//b.print_board();
			//std::cout << "evaluate\n";
			// backpropagate the result back to root
			// this brings back b to be root_node again
			backpropagation(best_leaf, b);
			//b.print_board();
			//std::cout << "backprop\n";
		}
		int best_move = board::passing_index;
		float best_score = -10000;
		for (int child = 0; child < root_node->num_children; child++)
		{
			float curr_score = root_node->children[child]->wins / root_node->children[child]->playouts;
			if (b.get_side_to_move() == COLOR_WHITE)
			{
				curr_score *= -1;
			}
			if (curr_score > best_score)
			{
				best_score = curr_score;
				best_move = root_node->children[child]->move;
			}
		}
		score = best_score * 100 * ((b.get_side_to_move() == COLOR_WHITE)?-1:1);
		return best_move;
	}
}
