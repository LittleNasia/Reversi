#pragma once
#include "Utils.h"
#include "NN_accumulator.h"
#include "ClippedReLU.h"



struct playfield_bitboard
{
	bitboard white_bb;
	bitboard black_bb;
};

struct Move
{
	playfield_bitboard bb;
	int64_t forced_passes;
};

class Board
{
public:
	
	Board();
	static constexpr int rows = 8;
	static constexpr int cols = 8;
	static constexpr int passing_index = rows * cols;
	static constexpr int max_ply = 100;
	static inline constexpr unsigned int to_1d(const unsigned int row, const unsigned int col)
	{
		return row * cols + col;
	}
	//using 8 bit per move seems pretty nice storage-wise, allows to copy the entire moves-array so much faster
	using move_type = uint8_t;
	//this assumes that there will never be more than 32 moves in a position
	//32 is a pretty nice number, that 
	using moves_array = move_type[rows * cols / 2];

	//does NOT check for legality of the move (only to be used from search, in conjecture with get_moves())
	void do_move(const int square, const bool update_accumulator = true);
	//does check for legality of the move
	const bool do_move_is_legal(const int square, const bool update_accumulator = true);
	//if you want to make a move using coordinates, go ahead, define it first though
	const bool do_move(const Point target_square);
	//gets the available moves list and then makes the move, returning the move made
	int do_random_move();
	//does whatever the lowest index move is 
	int do_first_move();
	//returns the pointer to the available moves array
	const move_type* get_moves();
	void print_board();
	void new_game();
	void undo_move();

	const uint8_t get_playfield_config() const;
	const playfield_bitboard& get_board() const { return bb; }
	const int get_num_moves() const { return num_moves; }
	const int get_score() const { return __popcnt64(bb.black_bb) - __popcnt64(bb.white_bb); }
	const int is_over() const { return forced_passes > 1; }
	const int get_ply() const { return ply; }
	const Color get_side_to_move() const { return side_to_move; }
	//returns the output of the accumulator from the perspective of the current side to move 
	const int16_t* get_current_accumulator_output() const { return accumulator_history[ply].output[side_to_move]; }
private:
	void capture(uint8_t move, const bool update_accumulator);

	//stores an accumulator for each of the ply of the game 
	NN::NN_accumulator accumulator_history[Board::max_ply];
	Move move_history[max_ply];
	moves_array available_moves;
	playfield_bitboard bb;
	int64_t forced_passes;
	Color side_to_move;
	Color result;
	int ply;
	int num_moves;
	int score;
	
};
