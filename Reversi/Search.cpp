#include "Search.h"
#include "Utils.h"
#include "TT.h"
#include "Evaluate.h"
#include <unordered_map>
#include <iostream>
#include <chrono>


using namespace std::chrono;
#include "TT.h"
namespace search
{
	

	//TT transposition_table;
	thread_local TT transposition_table;
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
		std::memcpy(board, b.get_board(), sizeof(bitboard) * COLOR_NONE);
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

	inline constexpr void order_moves(const TT_entry& entry, const Board::moves_array moves, Board::moves_array move_indices, const int num_moves, const bool found_tt_entry, int depth = 0)
	{
		//TT move should be the absolute first move that should be searched
		//TT exists for a reason lol
		int heuristics_used = 0;
		if (found_tt_entry)
		{
			std::swap(move_indices[0], move_indices[entry.move]);
			heuristics_used++;
		}

		//put the corner moves first, or right after a TT move if such exists
		//this is under the assumption, that contesting for corners is good
		for (int index = heuristics_used; index < (num_moves); index++)
		{
			if ((1ULL << moves[move_indices[index]]) & corners)
			{
				//std::cout << (int)moves[move_indices[index]] << "/" << (int)moves[move_indices[num_moves - 1 - c_square_moves]] << "\n";
				//std::cout << (int)move_indices[index] << "\\" << (int)move_indices[num_moves - 1 - c_square_moves] << "    " << num_moves << "\n\n";
				std::swap(move_indices[index], move_indices[heuristics_used++]);
				//std::cout << (int)moves[move_indices[index]] << "\n";
			}
		}

		//use any other heuristics
		int c_square_moves = 0;
		bool has_62 = false;
		//order the moves by searching moves on C-squares last
		for (int index = heuristics_used; index < (num_moves - c_square_moves); index++)
		{
			if ((1ULL << moves[move_indices[index]]) & C_squares)
			{
				if (depth == 10)
				{
					//std::cout << (int)moves[move_indices[index]] << "/" << (int)moves[move_indices[num_moves - 1 - c_square_moves]] << "\n";
					//std::cout << (int)move_indices[index] << "\\" << (int)move_indices[num_moves - 1 - c_square_moves] << "    " << num_moves << "\n\n";
				}
				std::swap(move_indices[index], move_indices[num_moves - 1 - c_square_moves]);
				//std::cout << (int)moves[move_indices[index]] << "\n";
				if ((index) != (num_moves - 1 - c_square_moves))
				{
					index--;
				}
				c_square_moves++;
			}
		}
		//std::cout << "\n";

	}

	int search(Board& b, int depth, int alpha, int beta, SearchInfo& s, bool doNull = true)
	{
		
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
			//return rng::rng()%0xFF;
		}

		int best_move = 0;
		int best_score = -value_inf;
		int TT_flag = TT_entry::flag_alpha;


		Board::moves_array moves;
		//initialize the array that holds move indices, so that if a move ordering change occurs, it will be reflected in this array
		Board::moves_array move_indices = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

		std::memcpy(moves, b.get_moves(), sizeof(Board::moves_array));
		const int numMoves = b.get_num_moves();

		

		const unsigned long long key = hash(b);
		bool found = false;
		const auto& entry = transposition_table.get(key, found);

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

		//null move pruning
		if ((numMoves) && (doNull) && (depth >= 5))
		{
			b.do_move(Board::invalid_index);
			int value = -search(b, depth - 4, -beta, -beta + 1, s, false);
			b.undo_move();
			if (value >= beta)
			{
				return beta;
			}
		}

		//order the move indices
		order_moves(entry, moves, move_indices, numMoves, found, depth);

		bool searching_pv = true;
		int current_move = 0;
		//iterate over moves 
		while (true)
		{
			s.ply++;
			b.do_move(moves[move_indices[current_move++]]);
			

			//principal variation search
			int score;
			if (searching_pv)
			{
				score = -search(b, depth - 1, -beta, -alpha,s);
			}
			else
			{
				score = -search(b, depth - 1, -alpha - 1, -alpha,s);
				if (score > alpha && score < beta)
				{
					score = -search(b, depth - 1, -beta, -alpha,s);
				}
			}

			b.undo_move();
			s.ply--;

			if (score > best_score)
			{
				//we already incremented current_move, so we have to subtract 1
				best_move = move_indices[current_move - 1];
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
				}
			}

			//we don't check invalid_index (passing moves), unless it is the only possible move
			//if it's the only possible move, it has just been evaluated, which also means numMoves is equal to 0
			//we break if the next move is a passing move, or if there are no moves
			if (moves[move_indices[current_move]] == Board::invalid_index || (!numMoves))
			{
				break;
			}
		}
		transposition_table.store({ key, best_move, alpha, depth, TT_flag });
		return alpha;
	}

	int search_move(Board& b, int depth, bool print, int& score, SearchInfo& s)
	{
		if (b.is_over())
		{
			return 64;
		}
		auto start_iterative = high_resolution_clock::now();
		//iterative deepening
		s.ply = 0;
		for (int d = 1; d <= depth; d++)
		{
			auto start = high_resolution_clock::now();
			search(b, d, -value_inf, value_inf,s, false);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);

			
			unsigned long long key = hash(b);
			bool found;
			const auto& entry = transposition_table.get(key, found);
			int score = entry.score;
			if(print)
				std::cout << "depth: " << d << " moves : ";
			int num_moves = 0;
			
			while (true)
			{
				unsigned long long key = hash(b);
				found = false;
				const auto& entry = transposition_table.get(key, found);
				if (found)
				{
					const auto moves = b.get_moves();
					if (print)
						std::cout << (int)moves[entry.move] << " ";
					num_moves++;
					b.do_move(moves[entry.move]);
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
					<< ((float)search_duration_so_far.count() + 1) / 1000000 << "\n";;
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
		else
		{
			std::cout << "root pos not found in tt\n";
			std::cout << "key is " << key << "\n";
			std::cout << "entry key is " << entry.posKey << "\n";
		}

		//std::cout << "\nmove is " << (int)b.get_moves()[m] << " with index " << m << "\n";
		//std::cout << "\nscore is " << entry.score << "\n";
		//transposition_table.clear();

		//save the searched score, return the move
		return score= entry.score, b.get_moves()[best_move_index];
	}
}