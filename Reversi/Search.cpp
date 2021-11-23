#include "Search.h"
#include "Utils.h"
#include "TT.h"
#include "Evaluate.h"
#include "MovePicker.h"

#include <unordered_map>
#include <iostream>
#include <chrono>


using namespace std::chrono;
#include "TT.h"
namespace search
{

	unsigned long long Zoribst[64][2];
	unsigned long long ZoribstSideToMove[2];


	void init()
	{
		//save the rng state, it's different each time the program starts, but we actually want it to be constant this time
		const int rng_backup_x = rng::x;
		const int rng_backup_y = rng::y;
		const int rng_backup_z = rng::z;
		//constant seed so that the pos keys are always the same for the same position
		rng::x = 123456789;
		rng::y = 362436069;
		rng::z = 521288629;

		for (int index = 0; index < Board::rows * Board::cols; index++)
		{
			for (int color = 0; color < COLOR_NONE; color++)
			{
				Zoribst[index][color] = (((unsigned long long)rng::rng()) << 48) | (((unsigned long long)rng::rng()) << 16) ^ (((unsigned long long)rng::rng()));
			}
		}
		for (int color = 0; color < COLOR_NONE; color++)
		{
			ZoribstSideToMove[color] = (((unsigned long long)rng::rng()) << 48) | (((unsigned long long)rng::rng()) << 16) ^ (((unsigned long long)rng::rng()));
		}

		//bring back the rng state so that it actually is different each time we run the program
		rng::x = rng_backup_x;
		rng::y = rng_backup_y;
		rng::z = rng_backup_z;
	}

	const unsigned long long hash(const Board& b)
	{
		bitboard board[COLOR_NONE];
		std::memcpy(board, &b.get_board(), sizeof(bitboard) * COLOR_NONE);
		unsigned long long posKey = 0;
		for (int color = 0; color < COLOR_NONE; color++)
		{
			while (board[color])
			{
				//get the index of the highest bit and pop it
				const int index = _lzcnt_u64(board[color]) ^ 63;
				posKey ^= Zoribst[index][color];
				board[color] ^= (1ULL << index);
			}
		}
		posKey ^= ZoribstSideToMove[b.get_side_to_move()];
		return posKey;
	}

	int nodes = 0;

	//function is only to be called if the hash of the stored position matches
	//returns value_inf if can't return a value, otherwise value 
	inline constexpr int probeEntry(const TT_entry& entry, const int depth, const int alpha, const int beta)
	{
		if (entry.depth >= depth)
		{
			//the score is exact, we can just return it 
			if (entry.flag == TT_entry::flag_exact)
			{
				return entry.score;
			}
			else if ((entry.flag == TT_entry::flag_alpha)
				&& (entry.score <= alpha))
			{
				return alpha;
			}
			else if ((entry.flag == TT_entry::flag_beta)
				&& (entry.score >= beta))
			{
				return beta;
			}
		}
		return value_inf;
	}

	
	template<bool root = false>
	int search(Board& b, int depth, int alpha, int beta, SearchInfo& s, bool doNull = true)
	{
		if ((!(nodes % 1000)))
		{
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - s.search_start).count();
			if ((s.time) - duration < 0)
			{
				std::cout << s.time << " " << duration << "\n";
				s.interrupted = true;
				return 0;
			}
		}
		if (b.is_over())
		{
			nodes++;
			int score = b.get_score();
			if (score)
			{
				score = ((score > 0) ? value_win : -value_win);
				//side to move perspective
				return score * ((b.get_side_to_move() == COLOR_BLACK) ? 1 : -1);
			}
			else
			{
				return 0;
			}
		}
		else if (depth <= 0)
		{
			nodes++;
			const auto score = s.eval_function(b);
			return score;
		}

		int best_move = 0;
		int best_score = -value_inf;
		int TT_flag = TT_entry::flag_alpha;

		const unsigned long long key = hash(b);
		bool found = false;
		const auto& entry = transposition_table.get(key, found);

		int eval;
		//we found the entry, let's see if the stored position is a transposition, or just info from previous search
		if (found)
		{
			int score;
			//check if the information stored is useful (can we return it or not)
			//if we can't, we will still be able to use the information in the move ordering 
			if ((score = probeEntry(entry, depth, alpha, beta)) != value_inf)
			{
				return score;
			}
		}
		eval = s.eval_function(b);
		//s.eval_stack[depth] = eval;
		//const bool improving = root ? false : -s.eval_stack[depth + 1] < (eval - 60);
		
		//reverse futility pruning
		//speeds up the search but doesn't impact the result in any meaningful way
		if (
			(depth <= 6)
			&& (eval - reverse_futility_margin * depth >= beta)
			&& (std::abs(beta) < value_win)
		   )
		{
			return eval;
		}

		//ProbCut
		//static constexpr int larger_depth = 8;
		//static constexpr float T = 2;
		//static constexpr int lower_depth = 4;
		//static constexpr float a_linear = 1.01485012;
		//static constexpr float b_linear = 19.85;
		//static constexpr float sigma = 199.93;
		//if (depth == larger_depth)
		//{
		//	int bound = beta + 80;
		//	if (std::abs(bound) < value_win && eval > bound)
		//	{
		//		int value = -search<false>(b, lower_depth, -bound, -bound + 1, s);
		//		if (value >= bound)
		//		{
		//			return beta;
		//		}
		//	}
		//	/*bound = (-T * sigma + alpha - b_linear) / a_linear;
		//	if (std::abs(bound) < value_win)
		//	{
		//		int value = -search<false>(b, lower_depth, -bound - 1, -bound, s);
		//		if (value <= bound)
		//		{
		//			return alpha;
		//		}
		//	}*/

		//}
			

		//null move pruning
		/*
		if ((numMoves) && (doNull) && (depth > 2))
		{
			b.do_move(Board::passing_index);
			int value = -search<false>(b, depth - 5, -beta, -beta + 1, s, false);
			b.undo_move();
			if (value >= beta)
			{
				return beta;
			}
		}
		*/


		MovePicker mp(b, entry, found, s);

		bool searching_pv = true;
		int current_move = 0;
		int played = 0;
		//iterate over moves 
		while (true)
		{
			if (s.interrupted)
			{
				return 0;
			}
			s.ply++;
			const auto current_move = mp.get_move();
			if (mp.get_move_count() && (current_move == Board::passing_index))
			{
				break;
			}
			
			b.do_move(current_move);
			int extension = (current_move == Board::passing_index) ? 1 : 0;

			//principal variation search
			int score;
			//score = -search<false>(b, depth - 1 + extension, -beta, -alpha, s,doNull);
			
			if (searching_pv)
			{
				score = -search<false>(b, depth - 1, -beta, -alpha, s, doNull);
			}
			else
			{
				//late move reductions
				int reduction = 1;
				if ((depth > 4) && (played > mp.get_move_count() * 5/7))
				{
					//reduction += 1;
				}
				
				score = -search<false>(b, depth - reduction + extension, -alpha - 1, -alpha, s, doNull);
				//we have to research
				if (score > alpha && score < beta)
				{
					score = -search<false>(b, depth - 1 + extension, -beta, -alpha, s, doNull);
				}
			}
			played++;

			b.undo_move();
			s.ply--;

			if (score > best_score)
			{
				//we already incremented current_move, so we have to subtract 1
				best_move = current_move;
				best_score = score;

				if (score > alpha)
				{
					alpha = score;
					searching_pv = false;
					TT_flag = TT_entry::flag_exact;
					if (alpha >= beta)
					{
						transposition_table.store({ key, best_move, beta, depth, TT_entry::flag_beta });
						return beta;
					}
					//s.history_data[best_move] += depth * depth;
				}
			}
			if (!mp.get_move_count())
			{
				break;
			}
		}
		transposition_table.store({ key, best_move, alpha, depth, TT_flag }, root);
		return best_score;
	}
	constexpr bool Root = true;
	void aspiration_window(Board& b, int depth, int score, SearchInfo& s)
	{
		int alpha = -value_inf;
		int beta = value_inf;
		int delta = window_size;

		if (depth >= aspiration_window_depth)
		{
			alpha = std::max(-value_win-1, score - delta);
			beta = std::min(value_win+1, score + delta);
		}
		int failed_low = 0;
		int failed_high = 0;

		while (true)
		{
			search<Root>(b, depth, alpha, beta, s);
			int curr_score;
			unsigned long long key = hash(b);
			bool found;
			const auto& entry = transposition_table.get(key, found);
			curr_score = entry.score;
			if ((curr_score < beta) && (curr_score > alpha))
			{
				return;
			}
			else if (curr_score <= alpha)
			{
				//fail-low
				failed_low++;
				beta = (alpha + beta) / 2;
				alpha = std::max(-value_win - 1, alpha - delta);
				
				//std::cout << "fail low\n";
			}
			else if (curr_score >= beta)
			{
				failed_high++;
				beta = std::min(value_win+1, beta + delta);
				
				//std::cout << "fail high\n";
			}
			delta *= 1.5;
		}
	}

	int search_move(Board& b, int depth, bool print, int& score, SearchInfo& s)
	{
		if (b.is_over())
		{
			return 64;
		}
		const bool using_time = s.time != max_time;
		//always use 20% of the remaining time for move
		const int64_t time_for_move = ((using_time)? s.time * time_per_move_percentage / 100 : max_time);
		s.time = time_for_move;
		//std::cout << (float) time_for_move / 1000000 << "\n";
		auto start_iterative = high_resolution_clock::now();
		//iterative deepening
		s.ply = 0;
		int best_move = 0;
		int best_score = 0;
		for (int d = 1; d <= depth; d++)
		{
			auto start = high_resolution_clock::now();
			s.search_start = start;
			aspiration_window(b, d, best_score, s);
			auto stop = high_resolution_clock::now();
			
			//duration for current ply
			auto duration = duration_cast<microseconds>(stop - start);
			if (s.interrupted)
			{
				break;
			}
			//print information about search
			unsigned long long key = hash(b);
			bool found;
			const auto& entry = transposition_table.get(key, found);
			int score = entry.score;
			best_move = entry.move;
			best_score = score;
			if (print)
				std::cout << "depth: " << d << " moves : ";
			int num_moves = 0;
			//get moves from TT, make sure move is legal as well
			while (true)
			{
				unsigned long long key = hash(b);
				found = false;
				const auto& entry = transposition_table.get(key, found);
				if (found)
				{
					const auto moves = b.get_moves();
					if (print)
						std::cout << (int)entry.move << " ";
					num_moves++;
					b.do_move(entry.move);
				}
				else
				{
					break;
				}
				if (b.is_over())
				{
					break;
				}
			}
			for (int i = 0; i < num_moves; i++)
			{
				b.undo_move();
			}
			auto now = high_resolution_clock::now();
			auto search_duration_so_far = duration_cast<microseconds>(now - start_iterative);
			if (print)
				std::cout << " nodes: " << nodes << "  nodes per second: " << (int)((float)nodes / (((float)duration.count() + 1) / 1000000)) << "  score: " << score << "  time elapsed: "
				<< ((float)search_duration_so_far.count() + 1) / 1000000 << std::endl;;

			if (using_time)
			{
				//determine whether we can search one more ply deep
				const int64_t time_taken = search_duration_so_far.count();
				//assume that each new search will take up to 4 times more time
				const int64_t time_predicted = time_taken * depth_time_increase_multiplier;
				//check if we still have time to search again
				if ((time_for_move - (time_predicted)) <= 0 || s.interrupted)
				{
					s.time -= time_taken;
					break;
				}
			}
			
			nodes = 0;
			
		}

		unsigned long long key = hash(b);
		int best_move_index = 64;
		bool found = false;
		const auto& entry = transposition_table.get(key, found);
		if (found)
		{
			best_move_index = entry.move;
			//std::cout <<"\nentry move "<< entry.move << "\n";
		}
		//this should never happen, root pos will always be added last to the TT, and it will always have the highest priority
		//its depth is simply the highest possible and the same keys gets overwritten even with lower depth
		else if (!s.interrupted)
		{
			std::cout << "root pos not found in tt\n";
			std::cout << "key is " << key << "\n";
			std::cout << "entry key is " << entry.posKey << "\n";
		}
		s.interrupted = false;
		//std::cout << "\nmove is " << (int)b.get_moves()[m] << " with index " << m << "\n";
		//std::cout << "\nscore is " << entry.score << "\n";
		//transposition_table.clear();

		//save the searched score, return the move
		return score = entry.score, entry.move;
	}
}