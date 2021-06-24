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

inline constexpr bitboard up(const bitboard b)
{
	return  b << 8;
}
inline constexpr bitboard down(const bitboard b)
{
	return  b >> 8;
}
inline constexpr bitboard right(const bitboard b)
{
	//the hex literal cuts off the left side of the board so the rightmost piece doesn't appear on the left side
	return  (b & 0xFEFEFEFEFEFEFEFEULL) >> 1;
}
inline constexpr bitboard left(const bitboard b)
{
	//the hex literal cuts off the right side of the board so the leftmost piece doesn't appear on the right side
	return  (b & 0x7F7F7F7F7F7F7F7FULL) << 1;
}
inline constexpr bitboard left_up(const bitboard b)
{
	return  up(left(b));
}
inline constexpr bitboard left_down(const bitboard b)
{
	return  down(left(b));
}
inline constexpr bitboard right_up(const bitboard b)
{
	return  up(right(b));
}
inline constexpr bitboard right_down(const bitboard b)
{
	return  down(right(b));
}

uint32_t* Board::getMoves()
{
	const auto& own_bb = m_bb[side_to_move];
	const auto& enemy_bb = m_bb[side_to_move == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE];
	const auto& empty = ~(own_bb | enemy_bb);

	//test upwards
	for (int iteration = 0; iteration < rows - 1; iteration++)
	{
		left_down(own_bb);
	}



}