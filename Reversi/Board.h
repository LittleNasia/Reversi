#pragma once
#include "Utils.h"

class PiecePositionContainer;

struct Move
{
	bitboard black_bb;
	bitboard white_bb;
	int64_t forced_passes;
};

class Board
{
public:
	
	Board();
	static constexpr int32_t rows = 8;
	static constexpr int32_t cols = 8;
	static constexpr int32_t invalid_index = rows * cols;
	static inline constexpr unsigned int to_1d(const unsigned int row, const unsigned int col)
	{
		return row * cols + col;
	}
	using input_type = Eigen::Matrix<ScalarType, rows, cols * NN_input_channels>;
	//this assumes that there will never be more than 32 moves in a position
	//32 is a pretty nice number, that 
	using moves_array = uint8_t[rows * cols / 2];
	//does NOT check for legality of the move (only to be used from search, in conjecture with getMoves())
	void do_move(const int square);
	//does check for legality of the move
	const bool do_move_is_legal(const int square);
	//if you want to make a move using coordinates, go ahead, define it first though
	const bool do_move(const Point target_square);
	//gets the available moves list and then makes the move
	void do_random_move();
	//returns the pointer to the available moves array
	const uint8_t* getMoves();
	void print_board();
	void new_game();
	void undo_move();

	const bitboard* const get_board() const { return m_bb; }
	const int getNumMoves() const { return num_moves; }
	const int getScore() const { return __popcnt64(m_bb[COLOR_BLACK]) - __popcnt64(m_bb[COLOR_WHITE]); }
	const int isOver() const { return forced_passes > 1; }
	const int getPly() const { return m_ply; }
	const Color getSideToMove() const { return side_to_move; }
private:
	void capture(uint8_t move);

	input_type m_nn_input;
	Move move_history[100];
	moves_array available_moves;
	bitboard m_bb[COLOR_NONE];
	int64_t forced_passes;
	Color side_to_move;
	Color result;
	int m_ply;
	int num_moves;
	int m_score;
	
};
