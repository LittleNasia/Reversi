#pragma once
#include "Board.h"
#include "Evaluate.h"

#include <functional>


namespace search
{
	constexpr int value_inf = 100000;
	constexpr int value_win = value_inf / 10;
	struct SearchInfo
	{
		int ply;
		std::function<int(Board&)> eval_function = evaluate;
	};
	void init();
	int search_move(Board& b, int depth, bool print,int& score, SearchInfo& s);
	const unsigned long long hash(const Board& b);
}
