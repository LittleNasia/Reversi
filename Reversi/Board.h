#pragma once
#include "Utils.h"

class PiecePositionContainer;

struct Move
{
	bitboard black_bb;
	bitboard white_bb;
};

class Board
{
public:
	Board();
	static constexpr uint32_t rows = 8;
	static constexpr uint32_t cols = 8;
	static constexpr uint32_t invalid_index = rows * cols;
	static inline constexpr unsigned int to_1d(const unsigned int row, const unsigned int col)
	{
		return row * cols + col;
	}
	using input_type = Eigen::Matrix<ScalarType, rows, cols * NN_input_channels>;
	//does NOT check for legality of the move (only to be used
	void do_move(int square);
	//does check for legality of the move
	bool do_move_is_legal(int square);
	bool do_move(Point target_square);
	//gets the available moves list and then makes the move
	void do_random_move();
	uint8_t* getMoves();
	void printBoard();
	void newGame();
	void undoMove();
	int getScore() { return __popcnt64(m_bb[COLOR_BLACK]) - __popcnt64(m_bb[COLOR_WHITE]); }
	int isOver() { return result != COLOR_NONE; }
private:
	bitboard m_bb[COLOR_NONE];
	void capture(uint8_t move);
	input_type m_nn_input;
	Color side_to_move;
	Color result;
	int m_ply;
	//this assumes that there will never be more than 32 moves in a position
	uint8_t available_moves[rows * cols / 2];
	int num_moves;
	int forced_passes;
	Move move_history[60];
};
