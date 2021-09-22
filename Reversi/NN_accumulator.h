#pragma once
#include "Utils.h"
//clude <Python.h>

class Board;

namespace NN
{
	//inspired by NNUE, however much simpler
	//uses int8 weights, or at least weights that have int8 precision while being int16, clipped ReLU in range 0..1, 
	struct NN_accumulator
	{
		int16_t output[COLOR_NONE][layer_sizes[1]];
		inline static int16_t weights[layer_sizes[0]][layer_sizes[1]];
		inline static int16_t biases[layer_sizes[1]];
		inline static bool weights_loaded = false;
		uint8_t used_config = 0;

		NN_accumulator();

		static void read_weights(float* weights_from_file, float* biases_from_file);

		void reset();

		//side_to_move is the side that is *making* the move, not the side that is moving *after* the move has been played
		void update_accumulator(const NN_accumulator& old_acc, const bitboard added_pieces, const bitboard captured_pieces, const Color side_to_move,
			const Board& b);

		//computes the output as if it was a normal forward pass
		//only to be used as a test function
		void recompute_acc(int16_t* input);

		void refresh(bitboard white_bb, bitboard black_bb, uint8_t board_config);
	};

	//instance of accumulator which is zeroed out, 
	const inline NN_accumulator zero_acc;

}