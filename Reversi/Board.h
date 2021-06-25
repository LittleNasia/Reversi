#pragma once
#include "Utils.h"

class PiecePositionContainer;

struct Move
{
	Point pos;
	//bitmasks that determine in which directions the move captured
	uint32_t moveDirectionMasks;
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
	bool do_move(int square);
	bool do_move(Point target_square);
	uint8_t* getMoves();
	void printBoard();
	void newGame();
private:
	bitboard m_bb[COLOR_NONE];
	input_type m_nn_input;
	Color side_to_move;
	//this assumes that there will never be more than 32 moves in a position
	uint8_t available_moves[rows * cols / 2];
	int m_ply;
};
