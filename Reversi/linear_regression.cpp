#include "linear_regression.h"
#include "search.h"

#include <algorithm>
#include <fstream>

namespace linear_regression
{
	float evaluate_psqt(bitboard bb, const float* weights)
	{
		float out = 0;
		while (bb)
		{
			int index = _lzcnt_u64(bb) ^ 63;
			bb ^= (1ULL << index);
			out += weights[index];
		}
		return out;
	}
	void linear_regression::load_weights()
	{
		std::ifstream linear_regression_weights("linear.bin", std::ios::binary);
		linear_regression_weights.read((char*)weights, sizeof(weights));
		linear_regression_weights.close();
	}
	int16_t evaluate(const board& b)
	{
		const auto& playfield = b.get_board();
		const bitboard own_bb = b.get_side_to_move() == COLOR_WHITE ? playfield.white_bb : playfield.black_bb;
		const bitboard opposite_bb = b.get_side_to_move() == COLOR_WHITE ? playfield.black_bb : playfield.white_bb;
		const bitboard available_moves = b.get_moves_bitmask();

		const int side_to_move_piece_count  = __popcnt64(own_bb);
		const int opposite_side_piece_count = __popcnt64(opposite_bb);

		float value = bias;
		value += evaluate_psqt(own_bb, side_to_move_weights);
		value += evaluate_psqt(opposite_bb, opposite_side_weights);
		value += evaluate_psqt(available_moves, available_moves_weights);
		int return_val = value * 100;
		return_val = std::clamp(return_val, -search::value_win + 1, search::value_win - 1);
		return return_val;
	}
}
