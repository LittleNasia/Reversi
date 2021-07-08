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
	TT transposition_table;
	unsigned long long Zoribst[64][2];
	unsigned long long ZoribstSideToMove[2];
	constexpr int value_inf = 1000000;
	constexpr int value_win = value_inf / 10;

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
				Zoribst[index][color] = (((unsigned long long)rng::rng()) << 32) | ((unsigned long long)rng::rng());
			}
		}
		for (int color = 0; color < COLOR_NONE; color++)
		{
			ZoribstSideToMove[color] = (((unsigned long long)rng::rng()) << 32) | ((unsigned long long)rng::rng());
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
		posKey ^= ZoribstSideToMove[b.getSideToMove()];
		return posKey;
	}

	int nodes = 0;

	//function is only to be called if the hash of the stored position matches
	//returns value_inf if can't return early, otherwise can 
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
			else if((entry.flag == TT_entry::flag_beta)
				&& (entry.score >= beta))
			{
				return beta;
			}
		}
		return value_inf;
	}

	int search(Board& b, int depth, int alpha, int beta)
	{
		if (b.isOver())
		{
			int score = b.getScore();
			if (score)
			{
				score = ((score > 0) ? value_win : -value_win);
				//side to move perspective
				return score * ((b.getSideToMove() == COLOR_BLACK) ? 1 : -1);
			}
			else
			{
				return 0;
			}
		}
		else if (depth == 0)
		{
			nodes++;
			const auto score = evaluate(b);
			return score;
			//return rng::rng()%0xFF;
		}	
		Board::moves_array moves;
		const int numMoves = b.getNumMoves();
		std::memcpy(moves, b.getMoves(), sizeof(Board::moves_array));
		int current_move = 0;
		int best_move = 0;
		int best_score = -value_inf;
		int TT_flag = TT_entry::flag_alpha;
		const unsigned long long key = hash(b);
		bool found = false;
		const auto& entry = transposition_table.get(key, found);
		//we found the entry, let's see if the stored position is a transposition, or just info from previous search
		if (found)
		{
			int score;
			//the entry has a higher or the same depth, meaning it is more important than whatever we have now, maybe we can do something with it
			if ((score =probeEntry(entry, depth, alpha, beta)) != value_inf)
			{
				return score;
			}
			//we couldn't return a value, so the entry sadly doesn't help our case, let's at least use the move ordering information
			std::swap(moves[0], moves[entry.move]);
		}
		
		while(true)
		{
			b.do_move(moves[current_move++]);
			const int score = -search(b, depth - 1, -beta, -alpha);
			b.undo_move();
			if (score > best_score)
			{
				//if we did find the move in TT (and have changed the move order)
				//we have to make sure we save the index of the correct move
				//as the move with index entry.move is now index 0, and vice versa
				
				if (found)
				{
					//we've found a TT move, if we're at index 0, we're actually at index entry.move
					if (current_move-1 == 0)
					{
						best_move = entry.move;
					}
					else if (current_move - 1 == entry.move)
					{
						best_move = 0;
					}
					else
					{
						best_move = current_move - 1;
					}
				}
				else
				{
					//save the index of the move, can speed up the move ordering 
					//since we can just swap the first index with best move index, instead of looking for
					//which index the best move is
					best_move = current_move - 1;
				}
				best_score = score;
			}
			if (score > alpha)
			{
				alpha = score;
				TT_flag = TT_entry::flag_exact;
			}
			if (alpha >= beta)
		 	{
				transposition_table.store({ key, best_move, beta, depth, TT_entry::flag_beta });
				return beta;
			}
			//we don't check invalid_index (passing moves), unless it is the only possible move
			//if it's the only possible move, it has just been evaluated, which also means numMoves is equal to 0
			//we break if the next move is a passing move, or if there are no moves
			if (moves[current_move] == Board::invalid_index || (!numMoves))
			{
				break;
			}
		}
		transposition_table.store({ key, best_move, alpha, depth, TT_flag });
		return alpha;
	}

	int search_move(Board& b, int depth)
	{
		if (b.isOver())
		{
			std::cout <<  " the board is finished, returning ";
			return 64;
		}
		//iterative deepening
		for (int d = 1; d <= depth; d++)
		{
			auto start = high_resolution_clock::now();
			search(b, d, -value_inf, value_inf);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			int num_moves = 0;
			std::cout << "depth: " << d <<  " moves : ";
			while (true)
			{
				unsigned long long key = hash(b);
				bool found = false;
				const auto& entry = transposition_table.get(key, found);
				if (found)
				{
					std::cout << (int)b.getMoves()[entry.move] << " ";
					num_moves++;
					b.do_move(b.getMoves()[entry.move]);
				}
				else
				{
					break;
				}
			}
			for (int i = 0; i < num_moves; i++)
			{
				b.undo_move();
			}
			std::cout << " nodes: " << nodes << " nodes per second: " << (int)((float)nodes/ (((float)duration.count()+1) / 1000000)) << "\n";;
			nodes = 0;
		}
		
		
		unsigned long long key = hash(b);
		int m = 64;
		bool found = false;
		const auto& entry = transposition_table.get(key, found);
		if (found)
		{
			m = entry.move;
			//std::cout <<"\nentry move "<< entry.move << "\n";
		}
		else
		{
			std::cout << "root pos not found in tt\n";
			std::cout << "key is " << key << "\n";
			std::cout << "entry key is " << entry.posKey << "\n";
		}
		
		std::cout << "\nmove is " << (int)b.getMoves()[m] << " with index " << m << "\n";
		std::cout << "\nscore is " << entry.score << "\n";
		transposition_table.clear();
		return b.getMoves()[m];
	}
}