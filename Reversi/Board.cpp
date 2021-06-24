#include "Board.h"


Board::Board()
{
	newGame();
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


uint32_t* Board::getMoves()
{
	const auto& own_bb = m_bb[side_to_move];
	const auto& enemy_bb = m_bb[side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE];
	const auto& empty = ~(own_bb | enemy_bb);

	//test upwards
	for (int iteration = 0; iteration < rows - 1; iteration++)
	{

	}



}