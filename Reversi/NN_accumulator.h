#pragma once
#include "Utils.h"

namespace NN
{
	inline int8_t weights[layer_sizes[0]][layer_sizes[1]];
	//inspired by NNUE, however much simpler 
	//uses int8 weights, clipped ReLU in range 0..1, 
	struct NN_accumulator
	{
		int16_t output[COLOR_NONE][layer_sizes[1]];
		

		NN_accumulator()
		{
			reset();
		}

		void reset()
		{
			//set everything to zero
			std::memset(output, 0, sizeof(output));
			//the board starts as empty before 0 ply, we put weights of empty square features to output
			for (int index = 0; index < 64; index++)
			{
				for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
				{
					output[COLOR_BLACK][neuron] += weights[index + 64 * 3][neuron];
					output[COLOR_WHITE][neuron] += weights[index + 64 * 3][neuron];
				}
			}
		}

		void update_accumulator(const NN_accumulator& old_acc, const bitboard added_pieces, const bitboard captured_pieces, const Color side_to_move)
		{
			//std::memcpy(output, old_acc.output, sizeof(output));
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


			//we now update the prespective independent inputs, combined white/black features and empty square features
			//the piece that was just placed is the only addition to combined input
			//it's also the only square that gets taken away from the empty squares input
			bitboard new_move = added_pieces & ~captured_pieces;
			const int new_move_index = _lzcnt_u64(new_move) ^ 63;
			for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
			{
				//here we use the index + 64*2, because taken square features are the third set of features (each set has size 64)
				output[side_to_move][neuron] += weights[new_move_index + 64 * 2][neuron];
				output[opposite_side][neuron] += weights[new_move_index + 64 * 2][neuron];

				//here we use the index + 64*3, because free square features are the fourth set of features (each set has size 64)
				output[side_to_move][neuron] -= weights[new_move_index + 64 * 3][neuron];
				output[opposite_side][neuron] -= weights[new_move_index + 64 * 3][neuron];
			}
		}
	};

}