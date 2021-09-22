#include "GameGenerator.h"
#include <future>
#include <fstream>
#include <memory>



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
		//start a game
		search::SearchInfo s;
		s.eval_function = evaluate;
		current_game.scored = !use_random_movers && save_scores;

		while (!b.is_over())
		{
			scored_move curr_move;
			int random_moves_done = 0;
			//does the game have to feature random moves only? no scores if that's the case 
			if (use_random_movers)
			{
				curr_move.move = b.do_random_move();
			}
			//each move is done using search, with a small chance of a random move
			else
			{
				//we search so that we have the move and score
				//in case the move gets changed to a random move, eval will still be there and it will still be correct
				int score = 0;
				curr_move.move = search::search_move(b, search_depth, false, score, s);
				if (save_scores)
				{
					curr_move.score = score * (lambda)/100;
				}
				//do a random move? 
				if (
					(random_move_chance > (rng::rng() & 1028))
					&& (b.get_ply() <= max_random_move_ply)
					&& (random_moves_done < max_random_moves)
					)
				{
					curr_move.move = b.do_random_move();
					random_moves_done++;
				}
				//don't do a random move, do the searched move
				else
				{
					b.do_move(curr_move.move);
				}
			}
			//save the move
			current_game.moves[current_game.game_moves++] = curr_move;
		}

		if (current_game.scored)
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
			int previous_index = 0;
			int previous_move = current_game.moves[previous_index].move;
			int move = current_game.moves[index].move;
			//make sure result also gets added based on the side to move perspective
			int result_multiplier = 1;
			//move will be equal to the previous move only if both are passing moves, as that's the only index allowed to repeat
			//that signals the end of the game
			while (move != previous_move)
			{
				current_game.moves[previous_index].score += (100 - lambda) * (result * result_multiplier) / 100;
				result_multiplier *= -1;
				previous_move = current_game.moves[previous_index].move;
				move = current_game.moves[index].move;
				index++;
				previous_index = index-1;
			}
			current_game.moves[previous_index].score += (100 - lambda) * (result * result_multiplier) / 100;
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
	return combined_games;
}

std::string GameGenerator::save_to_file(const std::vector<Game>& games)
{
	bool scored = games[0].scored;
	std::string filename = "games" + std::to_string(current_file_index++) + (scored ? ".sbin" : ".nsbin");
	std::ofstream game_file(filename, std::ios::binary);

	for (const auto& game : games)
	{
		int8_t previous_move = Board::invalid_index;
		for (const auto& move : game.moves)
		{
			game_file.write((char*)&move.move, sizeof(move.move));
			if (scored)
			{
				game_file.write((char*)&move.score, sizeof(move.score));
			}

			//two moves repeated, only passing moves can repeat, thus the game has ended
			if (move.move == previous_move)
			{
				break;
			}
		}
	}
	game_file.close();
	return filename;
}


void GameGenerator::convert_to_input_type(const std::string& filename)
{
	std::ifstream game_file(filename, std::ios::binary);
	bool scored = filename.find(".nsbin") == std::string::npos;

	//get file length
	game_file.seekg(0, game_file.end);
	int length = game_file.tellg();
	game_file.seekg(0, game_file.beg);

	//read the entire file
	auto buff = std::make_unique<char[]>(length);
	game_file.read(buff.get(), length);
	game_file.close();

	std::ofstream game_file_with_inputs(filename + "." + (scored ? "sinput" : "nsinput"), std::ios::binary);
	Board b;
	for (int move = 0; move < length; move++)
	{
		int8_t current_move = buff[move];
		const auto side_to_move = b.get_side_to_move();
		const auto& playfield = b.get_board();

		//save the current input, side to move perspective
		const auto side_to_move_bb = side_to_move == COLOR_WHITE ? playfield.white_bb : playfield.black_bb;
		game_file_with_inputs.write((char*)&side_to_move_bb, sizeof(side_to_move_bb));
		const auto opposite_side_bb = side_to_move == COLOR_WHITE ? playfield.black_bb : playfield.white_bb;
		game_file_with_inputs.write((char*)&opposite_side_bb, sizeof(opposite_side_bb));

		//save the score
		if (scored)
		{
			//this gets the memory of buff[move+1], which is where the scores start
			//then it casts it to int16_t array, as that's the score type
			//finally, it dereferences the pointer to get the score
			int16_t score = *((int16_t*)(&(buff[move + 1])));
			game_file_with_inputs.write((char*)&score, sizeof(score));
			move += 2;
		}
		b.do_move(current_move,false);
		if (b.is_over())
		{
			b.new_game();
		}
	}
	game_file_with_inputs.close();
}