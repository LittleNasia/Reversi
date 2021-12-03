#pragma once
#include "board.h"
#include "matrix.h"

namespace CNN
{
	class cnn_input_layer
	{
	public:
		using input_type = board;
		using output_type = matrix<board::rows, board::cols>;

		output_type* prepare_output(const input_type& b);
	private:
		void fill_plane(bitboard bb, int channel);
		output_type m_output[input_channels];
	};
}