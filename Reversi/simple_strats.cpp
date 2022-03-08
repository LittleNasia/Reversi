#include "simple_strats.h"
#include "move_picker.h"


int simple_strats::capture_the_most(board& b)
{
	int best_move = board::passing_index;
	int best_score = -100;
	color side_to_move = b.get_side_to_move();
	

	move_picker mp(b);

	while (mp.get_move_count())
	{
		const bitboard side_to_move_bb = ((side_to_move == COLOR_WHITE) ? b.get_board().white_bb : b.get_board().black_bb);
		const bitboard opposite_side_bb = ((side_to_move != COLOR_WHITE) ? b.get_board().white_bb : b.get_board().black_bb);
		int score = __popcnt64(side_to_move_bb) - oppo
	}
}