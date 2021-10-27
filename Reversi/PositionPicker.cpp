#include "PositionPicker.h"



bool PositionPicker::enter_and_get_position(const bitboard side_to_move_bb, const bitboard opposite_side_bb, pos_entry& ret_val)
{
	int index = rng::rng() % size;
	int score = ret_val.score;
	bool found = false;
	if (_positions[index].valid)
	{
		ret_val = _positions[index];
		found = true;
	}

	pos_entry&& curr_pos_entry = get_symmetries(side_to_move_bb, opposite_side_bb);
	curr_pos_entry.score = score;
	_positions[index] = curr_pos_entry;
	return found;
}


pos_entry PositionPicker::get_symmetries(const bitboard side_to_move_bb, const bitboard opposite_side_bb)
{
	pos_entry ret_val;
	ret_val.valid = true;
	static constexpr int side_to_move = 0;
	static constexpr int opposite_side = 1;
	static constexpr int side_none = 2;
	const bitboard const bb_templates[2] = { side_to_move_bb, opposite_side_bb };
	for (int symmetry = SYMMETRY_ORIGINAL; symmetry < SYMMETRY_NONE; symmetry++)
	{
		for (int side = side_to_move; side < side_none; side++)
		{
			bitboard curr_bb = bb_templates[side];
			bitboard result = 0ULL;

			while (curr_bb)
			{
				//usually it would be xored with 63, however in this case, the lookup tables expect the index to be the number of leading zeroes
				//so it means no xor 63
				int index = _lzcnt_u64(curr_bb);
				int equivalent_index = board_indices_vertical_mirror[symmetry][index];
				result ^= (1ULL << equivalent_index);
				//we bring our xor63 back as we now work with real indices, not lookup table indices
				curr_bb ^= (1ULL << (index ^ 63));
			}
			ret_val.transformed_bitboards[symmetry][side] = result;
		}
	}
	return ret_val;
}