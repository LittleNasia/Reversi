#pragma once
#include <intrin.h>
#include <cstdint>
using bitboard = uint64_t;;

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
