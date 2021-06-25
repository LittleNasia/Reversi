#include "Board.h"
#include <iostream>

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
				std::cout << "T";
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
}

uint8_t* Board::getMoves()
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
	available_moves[move_index] = invalid_index;

	return available_moves;
}