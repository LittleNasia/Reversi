#include "Board.h"
#include <iostream>
#include <functional>

Board::Board()
{
	newGame();
}

void Board::printBoard()
{
	int index = 63;
	getMoves();
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
			if ((1ULL << index) & m_bb[COLOR_BLACK])
			{
				std::cout << "X";
			}
			else if ((1ULL << index) & m_bb[COLOR_WHITE])
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
}

void Board::newGame()
{
	side_to_move = COLOR_BLACK;
	m_nn_input.setZero();
	m_nn_input(3, 3) = 1;
	m_nn_input(4, 4) = 1;
	m_nn_input(3, 4 + cols) = 1;
	m_nn_input(4, 3 + cols) = 1;
	m_bb[COLOR_BLACK] = 0 | (1ULL << to_1d(3, 3)) | (1ULL << to_1d(4, 4));
	m_bb[COLOR_WHITE] = 0 | (1ULL << to_1d(3, 4)) | (1ULL << to_1d(4, 3));
	m_ply = 0;
	forced_passes = 0;
	result = COLOR_NONE;
	move_history[m_ply].white_bb = m_bb[COLOR_WHITE];
	move_history[m_ply].black_bb = m_bb[COLOR_BLACK];
	move_history[m_ply].forced_passes = 0;
}

const uint8_t* Board::getMoves()
{
	const auto& own_bb = m_bb[side_to_move];
	const auto& enemy_bb = m_bb[side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE];
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
	for (int i = 0; i < rows - 3; ++i)
		victims |= down(victims) & enemy_bb;
	result |= down(victims) & empty;

	victims = left(own_bb) & enemy_bb;
	for (int i = 0; i < cols - 3; ++i)
		victims |= left(victims) & enemy_bb;
	result |= left(victims) & empty;

	victims = right(own_bb) & enemy_bb;
	for (int i = 0; i < cols - 3; ++i)
		victims |= right(victims) & enemy_bb;
	result |= right(victims) & empty;

	
	constexpr int iterations = ((rows - 3) < (cols - 3)) ? (rows - 3) : (cols - 3);

	victims = left_down(own_bb) & enemy_bb;
	for (int i = 0; i < iterations; ++i)
		victims |= left_down(victims) & enemy_bb;
	result |= left_down(victims) & empty;

	victims = right_down(own_bb) & enemy_bb;
	for (int i = 0; i < iterations; ++i)
		victims |= right_down(victims) & enemy_bb;
	result |= right_down(victims) & empty;

	victims = left_up (own_bb)&enemy_bb;
	for (int i = 0; i < iterations; ++i)
		victims |= left_up(victims) & enemy_bb;
	result |= left_up(victims) & empty;

	victims = right_up(own_bb) & enemy_bb;
	for (int i = 0; i < iterations; ++i)
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
void Board::capture(const uint8_t move)
{
	if (move != invalid_index)
	{
		uint32_t move_masks = 0;
		auto& own_bb = m_bb[side_to_move];
		auto& enemy_bb = m_bb[side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE];
		const auto& empty = ~(own_bb | enemy_bb);
		own_bb ^= (1ULL << move);
		constexpr bitboard(*direction_functions[])(const bitboard) = {
			left_up,
			up,
			right_up,
			right,
			right_down,
			down,
			left_down,
			left };
		for (int direction = DIRECTION_UP_LEFT; direction < DIRECTION_NONE; direction++)
		{
			bitboard victims = 0ULL;
			bitboard move_square = (1ULL << move);
			for (int iteration = 0; iteration < capture_iteration_count[move][direction]; iteration++)
			{
				victims |= (move_square = direction_functions[direction](move_square)) & enemy_bb;
				if (move_square & own_bb)
				{
					enemy_bb ^= victims;
					own_bb ^= victims;
					break;
				}
				else if (move_square & empty)
				{
					break;
				}
			}
		}
		move_history[m_ply].forced_passes = 0;
	}
	else
	{
		forced_passes++;
		move_history[m_ply].forced_passes = forced_passes;
	}
	move_history[m_ply].white_bb = m_bb[COLOR_WHITE];
	move_history[m_ply].black_bb = m_bb[COLOR_BLACK];
	side_to_move = side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

void Board::do_move(const int square)
{
	m_ply++;
	capture(square);
}

const bool Board::do_move_is_legal(const int square)
{
	m_ply++;
	getMoves();
	int curr_move = available_moves[0];
	int index = 0;
	bool found = 0;
	//could use binary search, however the function is still only to be called by humans anyways
	//it's not speed critical 
	while (curr_move != invalid_index)
	{
		curr_move = available_moves[index++];
		if (curr_move = square)
		{
			found = true;
			break;
		}
	}
	if (found)
	{
		capture(square);
		return true;
	}
	else
	{
		std::cout << "illegal\n";
		return false;
	}
}

void Board::do_random_move()
{
	m_ply++;
	getMoves();
	if (num_moves)
		capture(available_moves[rng() % num_moves]);
	else
		capture(invalid_index);
}	

void Board::undoMove()
{
	forced_passes = std::max(forced_passes - 1, 0LL);
	side_to_move = side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
	m_ply--;
	m_bb[COLOR_WHITE] = move_history[m_ply].white_bb;
	m_bb[COLOR_BLACK] = move_history[m_ply].black_bb;
	forced_passes = move_history[m_ply].forced_passes;
}