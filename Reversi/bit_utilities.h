#pragma once

#include <intrin.h>
#include <cstdint>

using bitboard = uint64_t;

constexpr int board_rows = 8;
constexpr int board_cols = 8;

inline const int pop_bit(bitboard& bb)
{
	int index = _lzcnt_u64(bb) ^ 63;
	bb ^= (1ULL << index);
	return index;
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

struct point
{
	int row;
	int col;
};
namespace CNN
{
	constexpr const point to_2d_index(int index)
	{
		return {(board_rows - (index / board_rows + 1)), (board_cols - (index % board_cols + 1))};
	}
}

#include <iostream>
inline void print_bitboard(const bitboard bb)
{
	int index = 63;
	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			std::cout << "[";
			if ((1ULL << index) & bb)
			{
				std::cout << "X";
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