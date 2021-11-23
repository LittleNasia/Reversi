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
	constexpr int time_per_move_percentage = 20;
	constexpr int depth_time_increase_multiplier = 3;
	constexpr int64_t max_time = 0x7FFFFFFFFFFFFFFF;
	constexpr int max_depth = 256;
	struct SearchInfo
	{
		const int (*eval_function)(Board&);
		int ply;
		int64_t time = 10000000;
		bool interrupted = false;
		std::chrono::steady_clock::time_point search_start;
	};
	void init();
	int search_move(Board& b, int depth, bool print,int& score, SearchInfo& s);
	const unsigned long long hash(const Board& b);
}
