#pragma once
#include "Board.h"



namespace search
{
	void init();
	int search_move(Board& b, int depth, bool print,int& score);
	const unsigned long long hash(const Board& b);
}
