#include "Evaluate.h"


const int evaluate(Board& b)
{
	const bitboard black_bb = b.get_board().black_bb;
	const bitboard white_bb = b.get_board().white_bb;
	constexpr int tempo = 12;
	//black side to move perspective
	if (use_nnue)
	{
		return NN::be.Evaluate(b);
	}
	else
	{
		int white_moves=0;
		int black_moves=0;

		b.get_moves();
		if (b.get_side_to_move() == COLOR_BLACK)
		{
			black_moves = b.get_num_moves();
		}
		else
		{
			white_moves = b.get_num_moves();
		}
		b.do_move(Board::passing_index, false);
		b.get_moves();
		if (b.get_side_to_move() == COLOR_BLACK)
		{
			black_moves = b.get_num_moves();
		}
		else
		{
			white_moves = b.get_num_moves();
		}
		b.undo_move();
	
		//int total_score =  NN::be.Evaluate(b);

		//center 16 score
		int center_16_score = 0;//(__popcnt64(black_bb & center_16_bitmask) - __popcnt64(white_bb & center_16_bitmask)) * 1;

		// center 4 score 
		int center_4_score = 0;//(__popcnt64(black_bb & center_4_bitmask) - __popcnt64(white_bb & center_4_bitmask)) * 1;

		//corners score
		int corner_score = (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * 10;

		//C-squares punishment (smaller than corners, so that taking corners is always beneficial, but not C squares alone)
		int C_squares_punishment = 0;// (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * -3;

		//total_score += center_16_score + center_4_score + corner_score + C_squares_punishment;
		constexpr int tempo = 14;
		//side to move perspective
		float x = ((black_moves - white_moves + corner_score) * ((b.get_side_to_move() == COLOR_BLACK) ? 7 : -7)) + tempo;
		return std::tanh((x - 5) / 125) * 20;
	}
}

const int evaluate_classical (Board& b)
{
	const bitboard black_bb = b.get_board().black_bb;
	const bitboard white_bb = b.get_board().white_bb;
	constexpr int tempo = 12;
	//black side to move perspective
	if (false)
	{
		return NN::be.Evaluate(b);
	}
	else
	{
		int white_moves = 0;
		int black_moves = 0;

		b.get_moves();
		if (b.get_side_to_move() == COLOR_BLACK)
		{
			black_moves = b.get_num_moves();
		}
		else
		{
			white_moves = b.get_num_moves();
		}
		b.do_move(Board::passing_index, false);
		b.get_moves();
		if (b.get_side_to_move() == COLOR_BLACK)
		{
			black_moves = b.get_num_moves();
		}
		else
		{
			white_moves = b.get_num_moves();
		}
		b.undo_move();

		//int total_score =  NN::be.Evaluate(b);

		//center 16 score
		int center_16_score = 0;//(__popcnt64(black_bb & center_16_bitmask) - __popcnt64(white_bb & center_16_bitmask)) * 1;

		// center 4 score 
		int center_4_score = 0;//(__popcnt64(black_bb & center_4_bitmask) - __popcnt64(white_bb & center_4_bitmask)) * 1;

		//corners score
		int corner_score = (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * 10;

		//C-squares punishment (smaller than corners, so that taking corners is always beneficial, but not C squares alone)
		int C_squares_punishment = 0;// (__popcnt64(black_bb & corners) - __popcnt64(white_bb & corners)) * -3;

		//total_score += center_16_score + center_4_score + corner_score + C_squares_punishment;
		constexpr int tempo = 14;
		//side to move perspective
		float x = ((black_moves - white_moves + corner_score) * ((b.get_side_to_move() == COLOR_BLACK) ? 7 : -7)) + tempo;
		return std::tanh((x - 5) / 125) * 20;
	}
}
