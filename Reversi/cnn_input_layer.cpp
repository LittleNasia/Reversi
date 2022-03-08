#include "cnn_input_layer.h"
#include "Utils.h"

namespace CNN
{

	void cnn_input_layer::fill_plane(bitboard bb, int channel)
	{
		while (bb)
		{
			auto index = _lzcnt_u64(bb) ^ 63;
			const point&& coords = to_2d_index(index);

			m_output[channel](coords.row, coords.col) = 1;
			bb ^= (1ULL << index);
		}
	}

	cnn_input_layer::output_type* cnn_input_layer::prepare_output(const input_type& b)
	{
		std::memset(&m_output, 0, sizeof(m_output));

		color side_to_move = b.get_side_to_move();
		const bitboard side_to_move_bb = side_to_move == COLOR_WHITE ? b.get_board().white_bb : b.get_board().black_bb;
		const bitboard opposite_side_bb = side_to_move != COLOR_WHITE ? b.get_board().white_bb : b.get_board().black_bb;
		const bitboard both = side_to_move_bb | opposite_side_bb;
		const bitboard not_side_to_move_bb = ~side_to_move_bb;
		const bitboard not_opposite_side_bb = ~opposite_side_bb;
		const bitboard legal_moves_bb = b.get_moves_bitmask();

		fill_plane(side_to_move_bb, 0);
		fill_plane(opposite_side_bb, 1);
		fill_plane(both, 2);
		fill_plane(not_side_to_move_bb, 3);
		fill_plane(not_opposite_side_bb, 4);
		fill_plane(legal_moves_bb, 5);

		/*for (int channel = 0; channel < input_channels; channel++)
		{
			for (int row = 0; row < board::rows; row++)
			{
				for (int col = 0; col < board::cols; col++)
				{
					std::cout << "[" << ((m_output[channel](row, col)==0)?" ":"X") << "]";
				}
				std::cout << "\n";
			}
			std::cout << "\n";
		}*/

		return m_output;
	}
}
