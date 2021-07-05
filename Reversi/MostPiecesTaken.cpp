#include "MostPiecesTaken.h"
#include <algorithm>
#include <cstring>
#include <iostream>

const int MostPiecesTaken::makeMove(Board& b)
{
	//color black wants to maximize the difference between number of pieces, color white wants to minimize
	constexpr const int& (*score_function[])(const int&, const int&) = {
		std::min<int>,
		std::max<int>
	};
	uint8_t available_moves[Board::rows * Board::cols / 2];
	//initialize the best move as a pass move, in case we don't find any moves, we just pass
	int best_move = Board::invalid_index;
	int best_move_score = (-(Board::rows * Board::cols)) * ((b.getSideToMove() == COLOR_BLACK) ? -1:1);
	Color side_to_move = b.getSideToMove();
	std::memcpy(available_moves, b.getMoves(), Board::rows * Board::cols / 2);
	//while the move is not a pass of turn
	int curr_move = 0;
	while (available_moves[curr_move] != Board::invalid_index)
	{
		//do move
		b.do_move(available_moves[curr_move]);
		//update best score
		int new_score = score_function[side_to_move](best_move_score, b.getScore());
		//has the score changed? score function makes sure score can only change for the better
		if (new_score != best_move_score)
		{
			best_move = available_moves[curr_move];
			best_move_score = new_score;
			//std::cout << "found new best move which is " << best_move << "\n";
		}
		curr_move++;
		b.undoMove();
	}
	return best_move;
}