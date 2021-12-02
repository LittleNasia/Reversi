#include "move_picker.h"
#include "evaluate.h"


#include <iterator>

constexpr int base_weights[64] = { 500, -50, 100, 50, 50, 100, -50, 500,
-50, -50, 0, 0, 0, 0, -50, -50,
100, 0, 0, 0, 0, 0, 0, 100,
50, 0, 0, 0, 0, 0, 0, 50,
50, 0, 0, 0, 0, 0, 0, 50,
100, 0, 0, 0, 0, 0, 0, 100,
-50, -50, 0, 0, 0, 0, -50, -50,
500, -50, 100, 50, 50, 100, -50, 500, };

move_picker::move_picker(board& b, const tt_entry& entry, const bool found_tt_entry, const search::search_info& s)
{
	const auto moves = b.get_moves();
	move_count = b.get_num_moves();

	for (int move = 0; move < move_count; move++)
	{
		weighted_moves[move].move = moves[move];
		weighted_moves[move].weight = 500;
		auto& weight = weighted_moves[move].weight;
		//weight += base_weights[moves[move]];
		weight += (ply_move_probability[b.get_ply()][moves[move]] * 100);
		weight += (config_move_probability[b.get_playfield_config()][moves[move]] * 160);
		//weight += std::clamp(s.history_data[moves[move]]/10,0,50);
		if (found_tt_entry && (moves[move] == entry.move))
		{
			weight += 10000;
		}
	}
	constexpr auto ordering_function = [](const weighted_move& m1, const weighted_move& m2) { return m1.weight > m2.weight; };
	std::sort(std::begin(weighted_moves), std::begin(weighted_moves) + move_count, ordering_function);
}

move_picker::move_picker(board& b)
{
	const auto moves = b.get_moves();
	move_count = b.get_num_moves();

	for (int move = 0; move < move_count; move++)
	{
		weighted_moves[move].move = moves[move];
	}
}

uint16_t move_picker::get_move()
{
	if (current_move >= move_count)
	{
		return board::passing_index;
	}
	return weighted_moves[current_move++].move;
}