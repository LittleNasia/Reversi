#pragma once
#include "board.h"

enum symmetries
{
	SYMMETRY_ORIGINAL,
	SYMMETRY_HORIZONTAL_MIRROR,
	SYMMETRY_VERTICAL_MIRROR,
	SYMMETRY_CENTRAL,
	SYMMETRY_DIAGONAL,
	SYMMETRY_DIAGONAL_HORIZONTAL,
	SYMMETRY_DIAGONAL_VERTICAL,
	SYMMETRY_DIAGONAL_CENTRAL,
	SYMMETRY_NONE
};


constexpr int board_indices_vertical_mirror[SYMMETRY_NONE][64] = {
{
63, 62, 61, 60, 59, 58, 57, 56, 
55, 54, 53, 52, 51, 50, 49, 48, 
47, 46, 45, 44, 43, 42, 41, 40, 
39, 38, 37, 36, 35, 34, 33, 32, 
31, 30, 29, 28, 27, 26, 25, 24, 
23, 22, 21, 20, 19, 18, 17, 16, 
15, 14, 13, 12, 11, 10, 9, 8, 
7, 6, 5, 4, 3, 2, 1, 0, 
},
{
56, 57, 58, 59, 60, 61, 62, 63,
48, 49, 50, 51, 52, 53, 54, 55,
40, 41, 42, 43, 44, 45, 46, 47,
32, 33, 34, 35, 36, 37, 38, 39,
24, 25, 26, 27, 28, 29, 30, 31,
16, 17, 18, 19, 20, 21, 22, 23,
8, 9, 10, 11, 12, 13, 14, 15,
0, 1, 2, 3, 4, 5, 6, 7,
},

{
0, 1, 2, 3, 4, 5, 6, 7,
8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23,
24, 25, 26, 27, 28, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39,
40, 41, 42, 43, 44, 45, 46, 47,
48, 49, 50, 51, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61, 62, 63,
},

{
7, 6, 5, 4, 3, 2, 1, 0,
15, 14, 13, 12, 11, 10, 9, 8,
23, 22, 21, 20, 19, 18, 17, 16,
31, 30, 29, 28, 27, 26, 25, 24,
39, 38, 37, 36, 35, 34, 33, 32,
47, 46, 45, 44, 43, 42, 41, 40,
55, 54, 53, 52, 51, 50, 49, 48,
63, 62, 61, 60, 59, 58, 57, 56,
},


{
63, 55, 47, 39, 31, 23, 15, 7,
62, 54, 46, 38, 30, 22, 14, 6,
61, 53, 45, 37, 29, 21, 13, 5,
60, 52, 44, 36, 28, 20, 12, 4,
59, 51, 43, 35, 27, 19, 11, 3,
58, 50, 42, 34, 26, 18, 10, 2,
57, 49, 41, 33, 25, 17, 9, 1,
56, 48, 40, 32, 24, 16, 8, 0,
},

{
56, 48, 40, 32, 24, 16, 8, 0,
57, 49, 41, 33, 25, 17, 9, 1,
58, 50, 42, 34, 26, 18, 10, 2,
59, 51, 43, 35, 27, 19, 11, 3,
60, 52, 44, 36, 28, 20, 12, 4,
61, 53, 45, 37, 29, 21, 13, 5,
62, 54, 46, 38, 30, 22, 14, 6,
63, 55, 47, 39, 31, 23, 15, 7,
},

{
0, 8, 16, 24, 32, 40, 48, 56,
1, 9, 17, 25, 33, 41, 49, 57,
2, 10, 18, 26, 34, 42, 50, 58,
3, 11, 19, 27, 35, 43, 51, 59,
4, 12, 20, 28, 36, 44, 52, 60,
5, 13, 21, 29, 37, 45, 53, 61,
6, 14, 22, 30, 38, 46, 54, 62,
7, 15, 23, 31, 39, 47, 55, 63,
},

{
7, 15, 23, 31, 39, 47, 55, 63,
6, 14, 22, 30, 38, 46, 54, 62,
5, 13, 21, 29, 37, 45, 53, 61,
4, 12, 20, 28, 36, 44, 52, 60,
3, 11, 19, 27, 35, 43, 51, 59,
2, 10, 18, 26, 34, 42, 50, 58,
1, 9, 17, 25, 33, 41, 49, 57,
0, 8, 16, 24, 32, 40, 48, 56,
},
};


struct pos_entry
{
	bitboard transformed_bitboards[SYMMETRY_NONE][COLOR_NONE];
	int16_t score;
	bool valid = false;
};



class position_picker
{
public:
	static constexpr int size = 10000000;
	bool enter_and_get_position(const bitboard side_to_move_bb, const bitboard opposite_side_bb, pos_entry& ret_val);
	const pos_entry* get_data() const { return _positions; }

private:
	pos_entry get_symmetries(const bitboard side_to_move_bb, const bitboard opposite_side_bb);
	pos_entry _positions[size];
};

