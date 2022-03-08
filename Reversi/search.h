#pragma once
#include "board.h"
#include "evaluate.h"
#include "tt.h"

#include <chrono>
#include <functional>
namespace search
{
	//tt transposition_table;
	inline thread_local tt transposition_table;

	constexpr int value_inf = 10000;
	constexpr int value_win = value_inf / 10;
	constexpr int aspiration_window_depth = 5;
	constexpr int window_size = 40;
	constexpr int reverse_futility_margin = 120;
	constexpr int time_per_move_percentage = 100;
	constexpr int depth_time_increase_multiplier = 3;
	constexpr int64_t max_time = 0x7FFFFFFFFFFFFFFF;
	constexpr int max_depth = 256;
	struct search_info
	{
		const int (*eval_function)(board&);
		int ply;
		int64_t time = max_time;
		bool interrupted = false;
		std::chrono::steady_clock::time_point search_start;
	};
	void init();
	int search_move(board& b, int depth, bool print,int& score, search_info& s);
	const unsigned long long hash(const board& b);
}
