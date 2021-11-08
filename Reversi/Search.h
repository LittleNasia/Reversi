#pragma once
#include "Board.h"
#include "Evaluate.h"
#include "TT.h"

#include <chrono>
#include <functional>
namespace search
{
	//TT transposition_table;
	inline thread_local TT transposition_table;
	constexpr int value_inf = 10000;
	constexpr int value_win = value_inf / 10;
	constexpr int aspiration_window_depth = 5;
	constexpr int window_size = 80;
	constexpr int reverse_futility_margin = 70;
	struct SearchInfo
	{
		SearchInfo()
		{
			reset();
		}
		void reset()
		{
			std::memset(eval_stack, 0, sizeof(eval_stack));
		}
		int ply;
		int time = 10000000;
		bool interrupted = false;
		std::chrono::steady_clock::time_point search_start;
		const int (*eval_function)(Board&);
		int16_t eval_stack[65];
	};
	void init();
	int search_move(Board& b, int depth, bool print,int& score, SearchInfo& s);
	const unsigned long long hash(const Board& b);
}
