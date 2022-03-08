#include "evaluate.h"
#include "cnn.h"
#include "linear_regression.h"
#include "mlp.h"

inline thread_local CNN::cnn evaluator;

constexpr int mcts_eval_playthroughs = 1000;
const int mcts_eval(board& b)
{
	float total = 0;

	
	for (int game = 0; game < mcts_eval_playthroughs; game++)
	{
		// we keep track of how many extra moves were made 
		// it's to make sure the node_pos is left 
		// exactly as it was before calling this function
		int num_moves_to_undo = 0;
		// simulate the game with random moves
		while (!b.is_over())
		{
			b.do_random_move();
			num_moves_to_undo++;
		}
		// check the game result
		int score = b.get_score();
		float result = 0;
		if (score)
		{
			// positive score means black wins
			// negative score means white wins
			result = (score > 0) ? 1 : -1;
		}
		// undo all the moves made in simulation
		for (int move_undo =0; move_undo < num_moves_to_undo; move_undo++)
		{
			//b.print_board();
			b.undo_move();
		}
		total += result;
	}
	total /= mcts_eval_playthroughs;
	return total * ((b.get_side_to_move() == COLOR_BLACK) ? 100 : -100);
}

const int evaluate(board& b)
{
	const bitboard black_bb = b.get_board().black_bb;
	const bitboard white_bb = b.get_board().white_bb;
	constexpr int tempo = 12;
	//black side to move perspective
#if use_nnue
	return NN::be.evaluate(b);
#else
	//return 0;
	return MLP::mlp_evaluator.forward(b);
	//return linear_regression::evaluate(b);
	//return evaluator.evaluate(b) * 1000;
	//return evaluate_classical(b);
	//return mcts_eval(b);
#endif
}



const int evaluate_classical (board& b)
{
	const bitboard black_bb = b.get_board().black_bb;
	const bitboard white_bb = b.get_board().white_bb;
	constexpr int tempo = 12;
	//black side to move perspective
	{
		//int white_moves = __popcnt64(white_bb);
		//int black_moves = __popcnt64(black_bb);

		///*b.get_moves();
		//if (b.get_side_to_move() == COLOR_BLACK)
		//{
		//	black_moves = b.get_num_moves();
		//}
		//else
		//{
		//	white_moves = b.get_num_moves();
		//}
		//b.do_move(board::passing_index, false);
		//b.get_moves();
		//if (b.get_side_to_move() == COLOR_BLACK)
		//{
		//	black_moves = b.get_num_moves();
		//}
		//else
		//{
		//	white_moves = b.get_num_moves();
		//}
		//b.undo_move();*/

		////int total_score =  NN::be.evaluate(b);

		////center 16 score
		//int center_16_score = 0;//(__popcnt64(black_bb & center_16_bitmask) - __popcnt64(white_bb & center_16_bitmask)) * 1;

		//// center 4 score 
		//int center_4_score = 0;//(__popcnt64(black_bb & center_4_bitmask) - __popcnt64(white_bb & center_4_bitmask)) * 1;

		////corners score
		////int corner_score = (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * 10;

		////C-squares punishment (smaller than corners, so that taking corners is always beneficial, but not C squares alone)
		//int C_squares_punishment = 0;// (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * -3;

		////total_score += center_16_score + center_4_score + corner_score + C_squares_punishment;
		//constexpr int tempo = 14;
		////side to move perspective
		//float x = ((black_moves - white_moves) * ((b.get_side_to_move() == COLOR_BLACK) ? 7 : -7));
		return 0;
	}
}

