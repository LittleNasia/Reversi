#include "Board.h"
#include <iostream>
#include <functional>
#include "Search.h"

Board::Board()
{
	new_game();
}

void Board::print_board()
{
	int index = 63;
	get_moves();
	int x = available_moves[0];
	int moves_i = 0;
	while (x != invalid_index)
	{
		std::cout << x << " ";
		moves_i++;
		x = available_moves[moves_i];
	}
	std::cout << "\n";
	int move_index = 0;
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			std::cout << "[";
			if ((1ULL << index) & bb.black_bb)
			{
				std::cout << "X";
			}
			else if ((1ULL << index) & bb.white_bb)
			{
				std::cout << "O";
			}
			else if (available_moves[move_index] == index)
			{
				std::cout << " ";
				move_index++;
			}
			else
			{
				std::cout << " ";
			}
			std::cout << "]";
			index--;
		}
		std::cout << "\n";
	}
	std::cout << "poskey is : " << search::hash(*this) << "\n";
	std::cout << "current side to move is " << ((side_to_move == COLOR_BLACK) ? 'X' : 'O') << "\n";
}

void Board::new_game()
{
	side_to_move = COLOR_BLACK;
	bb.black_bb = 0 | (1ULL << to_1d(3, 3)) | (1ULL << to_1d(4, 4));
	bb.white_bb = 0 | (1ULL << to_1d(3, 4)) | (1ULL << to_1d(4, 3));
	ply = 0;
	forced_passes = 0;
	result = COLOR_NONE;
	move_history[ply].bb.white_bb = bb.white_bb;
	move_history[ply].bb.black_bb = bb.black_bb;
	move_history[ply].forced_passes = 0;
	accumulator_history[0].reset();
	//update the 0 ply accumulators, by giving them the startpos
	//we pass the current acc as the old state, because we just need it to be empty and we know that all of them are empty
	//accumulator_history[ply].update_accumulator(accumulator_history[ply],(1ULL << to_1d(3, 3)) | (1ULL << to_1d(4, 4)), 0ULL, COLOR_BLACK);
	//accumulator_history[ply].update_accumulator(accumulator_history[ply],(1ULL << to_1d(3, 4)) | (1ULL << to_1d(4, 3)), 0ULL, COLOR_WHITE);
}

const Board::move_type* Board::get_moves()
{
	const auto& own_bb = side_to_move == COLOR_WHITE ? bb.white_bb : bb.black_bb;
	const auto& enemy_bb = side_to_move == COLOR_WHITE ? bb.black_bb : bb.white_bb;
	const auto& empty = ~(own_bb | enemy_bb);

	//bitboard containing the bits for each legal move
	bitboard result = 0;

	//bitboard containing the bits of enemy_bb pieces that are next to our pieces and could (potentially) be flipped
	bitboard victims = 0;


	//test upwards
	//this gets all pieces directly above our own pieces
	victims = up(own_bb) & enemy_bb;
	//rows -3 because:
	//in the most iterations case possible, we start at the bottom (row 7)
	//the operation above puts us at row 6
	//we then do 5 iterations to move up to row 1
	//after the loop we move one more up to row 0
	//same logic applies to all other loops
	for (int iteration = 0; iteration < rows - 3; iteration++)
	{
		//get pieces directly above the victims (as they all will be flipped if between our pieces)
		victims |= up(victims) & enemy_bb;
	}
	//victims are enemy_bb pieces directly above our own, we get empty squares directly above the victims that we can "sandwich"
	//that gives us the moves
	result |= up(victims) & empty;
	
	//same logic applies to all other directions

	victims = down(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < rows - 3; iteration++)
		victims |= down(victims) & enemy_bb;
	result |= down(victims) & empty;

	victims = left(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < cols - 3; iteration++)
		victims |= left(victims) & enemy_bb;
	result |= left(victims) & empty;

	victims = right(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < cols - 3; iteration++)
		victims |= right(victims) & enemy_bb;
	result |= right(victims) & empty;

	
	constexpr int iterations = ((rows - 3) < (cols - 3)) ? (rows - 3) : (cols - 3);

	victims = left_down(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < iterations; iteration++)
		victims |= left_down(victims) & enemy_bb;
	result |= left_down(victims) & empty;

	victims = right_down(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < iterations; iteration++)
		victims |= right_down(victims) & enemy_bb;
	result |= right_down(victims) & empty;

	victims = left_up (own_bb)&enemy_bb;
	for (int iteration = 0; iteration < iterations; iteration++)
		victims |= left_up(victims) & enemy_bb;
	result |= left_up(victims) & empty;

	victims = right_up(own_bb) & enemy_bb;
	for (int iteration = 0; iteration < iterations; iteration++)
		victims |= right_up(victims) & enemy_bb;
	result |= right_up(victims) & empty;


	//convert bits to array of move indices
	unsigned long long move_index = 0;
	while (result)
	{
		available_moves[move_index] = _lzcnt_u64(result) ^ 63;
		result ^= (1ULL << available_moves[move_index++]);
	}
	num_moves = move_index;
	available_moves[move_index] = invalid_index;

	return available_moves;
}

//only to be called with a move that is legal for sure
void Board::capture(const uint8_t move, const bool update_accumulator)
{
	if (move != invalid_index)
	{ 
		forced_passes = 0;
		uint32_t move_masks = 0;
		auto& own_bb = side_to_move == COLOR_WHITE ? bb.white_bb : bb.black_bb;
		auto& enemy_bb = side_to_move == COLOR_WHITE ? bb.black_bb : bb.white_bb;
		const auto& empty = ~(own_bb | enemy_bb);
		own_bb ^= (1ULL << move);
		
		for (int direction = DIRECTION_UP_LEFT; direction < DIRECTION_NONE; direction++)
		{
			bitboard victims = 0ULL;
			bitboard move_square = (1ULL << move);
			for (int iteration = 0; iteration < capture_iteration_count[move][direction]; iteration++)
			{
				//could've been a switch, but I have a much better idea about what kind of assembly gets generated from ifs
				//should be identical but oh well, haven't studied switches too much 
				if (direction == DIRECTION_UP_LEFT)
				{
					move_square = left_up(move_square);
				}
				else if (direction == DIRECTION_UP)
				{
					move_square = up(move_square);
				}
				else if (direction == DIRECTION_UP_RIGHT)
				{
					move_square = right_up(move_square);
				}
				else if (direction == DIRECTION_RIGHT)
				{
					move_square = right(move_square);
				}
				else if (direction == DIRECTION_DOWN_RIGHT)
				{
					move_square = right_down(move_square);
				}
				else if (direction == DIRECTION_DOWN)
				{
					move_square = down(move_square);
				}
				else if (direction == DIRECTION_DOWN_LEFT)
				{
					move_square = left_down(move_square);
				}
				else if (direction == DIRECTION_LEFT)
				{
					move_square = left(move_square);
				}
				victims |= move_square & enemy_bb;
				if (move_square & own_bb)
				{
					enemy_bb ^= victims;
					own_bb ^= victims;
					if(update_accumulator)
						accumulator_history[ply].update_accumulator(accumulator_history[ply-1],victims | (1ULL << move), victims, side_to_move, *this);
					break;
				}
				else if (move_square & empty)
				{
					break;
				}
			}
		}
		move_history[ply].forced_passes = 0;
	}
	else
	{
		forced_passes++;
		move_history[ply].forced_passes = forced_passes;
		
	}
	side_to_move = side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
	move_history[ply].bb.white_bb = bb.white_bb;
	move_history[ply].bb.black_bb = bb.black_bb;
	
}

void Board::do_move(const int square, const bool update_accumulator)
{
	ply++;
	capture(square, update_accumulator);
}

const bool Board::do_move_is_legal(const int square, const bool update_accumulator)
{
	ply++;
	get_moves();
	int curr_move = available_moves[0];
	int index = 0;
	bool found = 0;
	//could use binary search, however the function is still only to be called by humans anyways
	//it's not speed critical 
	while (curr_move != invalid_index)
	{
		curr_move = available_moves[index++];
		if (curr_move == square)
		{
			found = true;
			break;
		}
	}
	if (found || (square == invalid_index))
	{
		capture(square, update_accumulator);
		return true;
	}
	else
	{
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";
		std::cout << "illegal\n";

		return false;
	}
}

int Board::do_random_move()
{
	ply++;
	get_moves();
	int move;
	if (num_moves)
	{
		move = rng::rng() % num_moves;
		capture(available_moves[move], true);
	}
	else
	{
		move = invalid_index;
		capture(move, true);
	}
	return available_moves[move];
}	

void Board::undo_move()
{
	forced_passes = std::max(forced_passes - 1, 0LL);
	side_to_move = side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
	ply--;
	bb.white_bb = move_history[ply].bb.white_bb;
	bb.black_bb = move_history[ply].bb.black_bb;
	forced_passes = move_history[ply].forced_passes;
}


const uint8_t Board::get_playfield_config() const
{
	static constexpr bitboard masks[] =
	{
		(1ULL << 10) | (1ULL << 17),
		(1ULL << 41) | (1ULL << 50),
		(1ULL << 46) | (1ULL << 53),
		(1ULL << 3) | (1ULL << 4),
		(1ULL << 24) | (1ULL << 32),
		(1ULL << 31) | (1ULL << 39),
		(1ULL << 60) | (1ULL << 59),
	};
	bitboard combined_bb = bb.white_bb | bb.black_bb;
	uint8_t result = 0;

	for (int bit_index = 0; bit_index < sizeof(uint8_t); bit_index++)
	{
		if ((combined_bb & masks[bit_index]) == masks[bit_index])
		{
			result |= (1 << bit_index);
		}
	}
	return result;

}