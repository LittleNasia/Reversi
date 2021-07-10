#pragma once
#include "Utils.h"

namespace NN
{

	//inspired by NNUE, however much simpler 
	//uses int8 weights, clipped ReLU in range 0..1, 
	struct NN_accumulator
	{
		uint16_t output[COLOR_NONE][layer_sizes[1]];
		int8_t weights[layer_sizes[0]][layer_sizes[1]];

		void update_accumulator(const bitboard added_pieces, const bitboard captured_pieces, const Color side_to_move)
		{
			const Color opposite_side = ((side_to_move == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK);
			//the input structure is as follows:
			//the first 64 neurons are the side_to_move pieces (the side that just made the move), followed by 64 other side pieces
			//the next 64 neurons are both colors combined and the last 64 are the empty squares

			//serialization of added pieces
			//side to move has just gained some pieces and has its own weights for the gained pieces
			//the opposite side to move has uses the same weights, but has different input

			//the added pieces use the "side to move pieces" weights for side_to_move
			//meanwhile they use "opposite side pieces" weights for opposite_side
			//both sides to move are calculated at the same time

			//it's always only side to move that gains pieces
			bitboard remaining_added_pieces = added_pieces;
			while (remaining_added_pieces)
			{
				const int bit_index = _lzcnt_u64(remaining_added_pieces) ^ 63;
				remaining_added_pieces ^= (1ULL << bit_index);
				for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
				{
					//the current side_to_move has just captured some pieces, good for them, the indices are normal
					output[side_to_move][neuron] += weights[bit_index][neuron];

					//current side_to_move pieces appear to be opposite_side from the perspective of the opposite_side
					//we use the bit_index + 64, as it's the one that corresponds to opponent_pieces input 
					output[opposite_side][neuron] += weights[bit_index + 64][neuron];
				}
			}

			//serialization of removed pieces
			//logic is the same, however this time we subtract the weights, as the features are just lost
			//it's only opposite side that loses pieces
			bitboard remaining_captured_pieces = captured_pieces;
			while (remaining_captured_pieces)
			{
				const int bit_index = _lzcnt_u64(remaining_captured_pieces) ^ 63;
				remaining_captured_pieces ^= (1ULL << bit_index);
				for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
				{
					//the opponent of current side_to_move has just lost pieces
					//we use the bit_index + 64, as it's the one that corresponds to opponent_pieces input 
					output[side_to_move][neuron] -= weights[bit_index + 64][neuron];

					//current opposite_side pieces appear to be their own from the perspective of the opposite_side
					output[opposite_side][neuron] -= weights[bit_index][neuron];
				}
			}

		}
	};

}