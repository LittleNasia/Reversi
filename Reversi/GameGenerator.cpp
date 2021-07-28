#include "GameGenerator.h"
#include <future>
#include <fstream>



GameGenerator::GameGenerator()
{
	reset();
}

std::vector<Game> GameGenerator::game_generator_worker(const bool use_random_movers, const bool save_scores, const int search_depth)
{
	std::vector<Game> games;
	Board b;
	for (int game = 0; game < games_per_file; game++)
	{
		if (!(game % (games_per_file / 60)))
		{
			std::cout << "progress " << game << "/" << games_per_file << "\n";
		}
		Game current_game;
		current_game.moves[0] = game_begin;
		//start a game
		search::SearchInfo s;
		s.eval_function = evaluate;
		current_game.scored = !use_random_movers && save_scores;

		while (!b.is_over())
		{
			int move;
			int score;
			int random_moves_done = 0;
			//does the game have to feature random moves only? no scores if that's the case 
			if (use_random_movers)
			{
				move = b.do_random_move();
				current_game.moves[++current_game.game_moves] = move;
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
				}
				//do a random move? 
				if (
					(random_move_chance > (rng::rng() & 1028))
					&& (b.get_ply() <= max_random_move_ply)
					&& (random_moves_done < max_random_moves)
					)
				{
					move = b.do_random_move();
					random_moves_done++;
				}
				else
				{
					b.do_move(move);
				}
				//save the move
				current_game.moves[++current_game.game_moves] = move;
			}
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
			int previous_move = current_game.moves[index - 1];
			int move = current_game.moves[index];
			//make sure result also gets added based on the side to move perspective
			int result_multiplier = 1;
			//move will be equal to the previous move only if both are passing moves, as that's the only index allowed to repeat
			//that signals the end of the game
			while (move != previous_move)
			{
				current_game.scores[index - 1] += (100 - lambda) * (result * result_multiplier) / 100;
				result_multiplier *= -1;
				previous_move = current_game.moves[index - 1];
				move = current_game.moves[index];
				index++;
			}
			//the last move wasn't scored, as it has ended the game and it wasn't searched
			//store the game result in it, as the game has just ended
			current_game.scores[index-1] = (result * result_multiplier);
		}
		games.emplace_back(current_game);
		b.new_game();
	}
	return games;
}


std::vector<Game> GameGenerator::generate_games(bool use_random_movers, bool save_scores, int search_depth, int num_threads)
{
	std::vector<std::future<std::vector<Game>>> thread_results(num_threads);
	std::vector<Game> combined_games;
	for (int current_thread = 0; current_thread < num_threads; current_thread++)
	{
		thread_results[current_thread] = std::async(&GameGenerator::game_generator_worker,this, use_random_movers, save_scores, search_depth);
	}

	for (int current_thread = 0; current_thread < num_threads; current_thread++)
	{
		const auto& result = thread_results[current_thread].get();
		combined_games.insert(combined_games.begin(), result.begin(), result.end());
	}
	std::ofstream games_file("games.bin", std::ios::binary);
	std::cout << "generated " << combined_games.size() << " games\n";
	for (auto& game : combined_games)
	{
		int index = 0;
		Board b;
		while (!b.is_over())
		{
			if (game.moves[index] != game_begin)
			{
				b.do_move(game.moves[index],false);
			}
			games_file.write((char*)&game.moves[index], 1);
			games_file.write((char*)&game.scores[index], 2);
			index++;
		}
	}
	games_file.close();
	return combined_games;
}


