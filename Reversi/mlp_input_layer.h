#pragma once
#include "board.h"

namespace MLP
{

	// just like with linear regression
	// the input is 64 features of side_to_move pieces
	// 64 features of opposite_side_pieces
	// and 64 features of available_moves 
	static inline constexpr int input_planes = 3;
	static inline constexpr int features_per_plane = 64;
	static inline constexpr int input_features = features_per_plane * input_planes;


	//this class takes the board as the input and makes the first forward pass 
	template <int out_neurons>
	class mlp_input_layer
	{
	public:
		static constexpr int side_to_move_plane_offset	= 0;
		static constexpr int opposite_side_plane_offset	= side_to_move_plane_offset	 + features_per_plane;
		static constexpr int moves_bitmask_plane_offset	= opposite_side_plane_offset + features_per_plane;
		using output_type = float;

		output_type* forward(const board& b)
		{
			std::memcpy(output, biases, sizeof(output));

			const playfield_bitboard& input = b.get_board();
			const color& side_to_move = b.get_side_to_move();

			const bitboard& own_bb		=	((side_to_move == COLOR_BLACK) ? input.black_bb : input.white_bb);
			const bitboard& opposite_bb =	((side_to_move == COLOR_BLACK) ? input.white_bb : input.black_bb);
			const bitboard& moves_bb	=	b.get_moves_bitmask();

			parse_input_plane(own_bb		, side_to_move_plane_offset);
			parse_input_plane(opposite_bb	, opposite_side_plane_offset);
			parse_input_plane(moves_bb		, moves_bitmask_plane_offset);

			return output;
		}

		void load_weights(float* loaded_weights, float* loaded_biases)
		{
			for (int col = 0, index = 0; col < input_features; col++)
			{
				for (int row = 0; row < out_neurons; row++, index++)
				{
					weights[row][col] = loaded_weights[index];
				}
			}
			std::memcpy(biases, loaded_biases, sizeof(biases));
		}
	private:
		// offset determines which input plane weights we're using
		inline void parse_input_plane(const bitboard bb, const int offset)
		{
			auto curr_bb = bb;
			while (curr_bb)
			{
				int index = pop_bit(curr_bb);
				for (int output_neuron = 0; output_neuron < out_neurons; output_neuron++)
				{				
					output[output_neuron] += weights[output_neuron][index + offset];
				}
			}
		}

		alignas(128) float output[out_neurons];
		alignas(128) float weights[out_neurons][input_features];
		alignas(128) float biases[out_neurons];
	};
}