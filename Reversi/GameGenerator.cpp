#include "GameGenerator.h"



GameGenerator::GameGenerator()
{
	reset();
}



std::vector<Game> GameGenerator::generate_games(const bool use_random_movers, const bool save_scores, const int search_depth, int num_threads)
{
	std::vector<Game> games;
	Board b;
	for (int game = 0; game < games_per_file; game++)
	{
		Game current_game;
		current_game.moves[0] = game_begin;
		//start a game
		search::SearchInfo s;
		s.eval_function = evaluate;
		while (!b.is_over())
		{
			int move;
			int score;
			int random_moves_done = 0;
			//does the game have to feature random moves only? no scores if that's the case 
			if (use_random_movers)
			{
				move = b.do_random_move();
			}
			//each move is done using search, with a small chance of a random move
			else
			{
				//we search so that we have the move and score
				//in case the move gets changed to a random move, eval will still be there and it will still be correct
				move = search::search_move(b, search_depth, false, score, s);
				if (save_scores)
				{
					current_game.scores[current_game.game_moves] = score * lambda / 100;
					current_game.scored=true;
				}
				//do a random move? 
				if ((random_move_chance > (rng::rng() & 1028))
					&& (b.get_ply() <= max_ply)
					&& (random_moves_done < max_random_moves)
					)
				{
					move = b.do_random_move();
				}
				else
				{
					b.do_move(move);
				}
				//save the move
				current_game.moves[++current_game.game_moves] = move;
			}
			current_game.moves[++current_game.game_moves] = move;
		}
		
		if (save_scores && !use_random_movers)
		{
			//get the result of the game 
			//each score saved in a game file is an interpolation between eval and game result
			int result = 0;
			if (b.get_score() > 0)
			{
				result = search::value_win;
			}
			else if (b.get_score() < 0)
			{
				result = -search::value_win;
			}
			int index = 1;
			int previous_move = current_game.moves[index-1];
			int move = current_game.moves[index];
			//move will be equal to the previous move only if both are passing moves, as that's the only index allowed to repeat
			//that signals the end of the game
			while (move != previous_move)
			{
				current_game.scores[index - 1] += (100 - lambda) * result / 100;
				index++;
				previous_move = current_game.moves[index - 1];
				move = current_game.moves[index];
			}
			//the last move wasn't scored, as it has ended the game and it wasn't searched
			//same the game result in it, as the game has just ended
			current_game.moves[index] = result;
		}
		games.emplace_back(current_game);
	}
	
}


