#include "NN_accumulator.h"
#include "Board.h"


NN::NN_accumulator::NN_accumulator()
{
	//set everything to zero
	std::memset(output, 0, sizeof(output));
}

void NN::NN_accumulator::read_weights(float* weights_from_file, float* biases_from_file)
{
	if (!weights_loaded)
	{
		for (int row = 0, index = 0; row < layer_sizes[0]; row++)
		{
			for (int col = 0; col < layer_sizes[1]; col++, index++)
			{
				//float weights are in range -128/64 to 127/64, however in the accumulator, they are directly contributing to the input
				//as the input is binary 0 or 1, it is not scaled by the input scaling factor
				//multiplying the weights each time they're added by 127 could very easily both overflow and be just slow
				//that's why the weights are scaled by both the input factor and weight factor, and just like normal output of each layer
				//weights are then divided by the weight scaling factor
				//as one can notice, this is identical to just using the input scaling factor for weights directly
				//however that would imply the int8 weights can only hold values from -128/127 (-128 divided by 127, scaling factor of input) to 127/127, which is not what we want
				//instead, I just expand the range of int values for weights to be -256, 255, which allows me to use both input scaling factor
				//and keep the weights in range -2,127/64
				int weight_int = (int)(weights_from_file[index] * weight_scaling_factor * input_scaling_factor / weight_scaling_factor);
				weight_int = std::clamp(weight_int, -256, 255);
				weights[row][col] = (int16_t)weight_int;
			}
		}
		for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
		{
			//biases are effectively weights, same rules apply
			//in other layers they are not effectively weights, but in this case they are
			int bias_int = (int)(biases_from_file[neuron] * weight_scaling_factor * input_scaling_factor / weight_scaling_factor);
			bias_int = std::clamp(bias_int, -256, 255);
			biases[neuron] = (int16_t)bias_int;
		}
		weights_loaded = true;
	}

}

void NN::NN_accumulator::reset()
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

//side_to_move is the side that is *making* the move, not the side that is moving *after* the move has been played
void NN::NN_accumulator::update_accumulator(const NN_accumulator& old_acc, const bitboard added_pieces, const bitboard captured_pieces, const Color side_to_move,
	const Board& b)
{
	std::memcpy(output, old_acc.output, sizeof(output));
	const Color opposite_side = ((side_to_move == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK);
	//the input structure is as follows:
	//the first 64 neurons are the side_to_move pieces (the side that just made the move), followed by 64 other side pieces
	//the next 64 neurons are both colors combined and the last 64 are the empty squares

	//depending on the configuration of 8 key squares, depending on whether they are taken or not, different weights are used
	//8 key squares having taken/not taken values leads to 256 different combinations


	//serialization of added pieces
	//side to move has just gained some pieces and has its own weights for the gained pieces
	//the opposite side to move has uses the same weights, but has different input

	//the added pieces use the "side to move pieces" weights for side_to_move
	//meanwhile they use "opposite side pieces" weights for opposite_side
	//both sides to move are calculated at the same time

	//it's always only side to move that gains pieces

	int curr_config = b.get_playfield_config();
	if (curr_config != used_config)
	{
		const auto& bbs = b.get_board();
		refresh(bbs.white_bb, bbs.black_bb, curr_config);
	}
	bitboard remaining_added_pieces = added_pieces;
	while (remaining_added_pieces)
	{
		const int bit_index = _lzcnt_u64(remaining_added_pieces) ^ 63;
		remaining_added_pieces ^= (1ULL << bit_index);
		for (int neuron = 0; neuron < layer_sizes[1]; neuron++)
		{
			//the current side_to_move has just captured some pieces, good for them, the indices are normal
			output[side_to_move][neuron] += weights[bit_index + (repeat * 64 * 4)][neuron];

			//current side_to_move pieces appear to be opposite_side from the perspective of the opposite_side
			//we use the bit_index + 64, as it's the one that corresponds to opponent_pieces input 
			output[opposite_side][neuron] += weights[bit_index + 64 + (repeat * 64 * 4)][neuron];
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
			output[side_to_move][neuron] -= weights[bit_index + 64 + (repeat * 64 * 4)][neuron];

			//current opposite_side pieces appear to be their own from the perspective of the opposite_side
			output[opposite_side][neuron] -= weights[bit_index + (repeat * 64 * 4)][neuron];
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
		output[side_to_move][neuron] += weights[new_move_index + 64 * 2 + (repeat * 64 * 4)][neuron];
		output[opposite_side][neuron] += weights[new_move_index + 64 * 2 + (repeat * 64 * 4)][neuron];

		//here we use the index + 64*3, because free square features are the fourth set of features (each set has size 64)
		output[side_to_move][neuron] -= weights[new_move_index + 64 * 3 + (repeat * 64 * 4)][neuron];
		output[opposite_side][neuron] -= weights[new_move_index + 64 * 3 + (repeat * 64 * 4)][neuron];
	}


}

//computes the output as if it was a normal forward pass
//only to be used as a test function
void NN::NN_accumulator::recompute_acc(int16_t* input)
{
	for (int output_node = 0; output_node < layer_sizes[1]; output_node++)
	{
		output[COLOR_WHITE][output_node] = biases[output_node];
		output[COLOR_BLACK][output_node] = biases[output_node];
	}

	std::cout << "\n";
	for (int input_node = 0; input_node < layer_sizes[0]; input_node++)
	{
		for (int output_node = 0; output_node < layer_sizes[1]; output_node++)
		{
			output[COLOR_BLACK][output_node] += input[input_node] * weights[input_node][output_node];
			output[COLOR_WHITE][output_node] += input[input_node] * weights[input_node][output_node];
		}
	}

}

void NN::NN_accumulator::refresh(bitboard white_bb, bitboard black_bb, uint8_t board_config)
{

}