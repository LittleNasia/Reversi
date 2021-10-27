#include "GameGenerator.h"
#include "PositionPicker.h"


#include <future>
#include <fstream>
#include <memory>



GameGenerator::GameGenerator()
{
	std::ifstream book_file("book.bin", std::ios::binary);
	book_file.read((char*)book, sizeof(book));
	book_file.close();
	reset();
}

std::vector<Game> GameGenerator::game_generator_worker(const bool use_random_movers, const bool save_scores, const int search_depth)
{
	std::vector<Game> games;
	Board b;
	for (int game = 0; game < games_per_file; game++)
	{
		int curr_book_index = rng::rng() % book_size;
		const auto* curr_book = book[curr_book_index];
		if (!(game % (games_per_file / 60)))
		{
			std::cout << "progress " << game << "/" << games_per_file << "\n";
		}
		Game current_game;
		current_game.moves.reserve(64);
		//start a game
		search::SearchInfo s;
		s.eval_function = evaluate;

		current_game.scored = !use_random_movers && save_scores;
		
		int random_moves_done = 0;
		bool previous_was_random = false;
		while (!b.is_over())
		{
			scored_move curr_move;
			const auto ply = b.get_ply();
			//std::cout << (int)b.get_playfield_config() << "\n";
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
				//std::cout << "\n\n";
				//std::cout << score << "\n";
				if (save_scores)
				{
					curr_move.score = score * (lambda) / 100;//NN::be.Evaluate(b);//
					//std::cout << curr_move.score << "\n";
				}
				int random_choice = rng::rng() % 1000;

				//do we have a book move? if we do, try to make that move
				if ((ply < 10) && (curr_book[ply] != -1))
				{
					b.do_move(curr_book[ply]);
				}
				//first move of the game is always random, but it doesn't count towards the counter
				else if (b.get_ply() == 0)
				{
					curr_move.move = b.do_random_move();
				}
				//do a normal random move? 
				else if 
					(
					(((random_move_chance - random_move_chance_reduction*ply) > (random_choice)))
					&& (b.get_ply() <= max_random_move_ply)
					&& (random_moves_done < max_random_moves)
					)
				{
					curr_move.move = b.do_random_move();
					random_moves_done++;
					previous_was_random = !previous_was_random;
				}
				//don't do a random move, do the searched move
				else
				{
					b.do_move(curr_move.move);
				}
			}
			//save the move
			current_game.moves.emplace_back(curr_move);
			//update stats
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

			//make sure result also gets added based on the side to move perspective
			int result_multiplier = 1;
			//move will be equal to the previous move only if both are passing moves, as that's the only index allowed to repeat
			//that signals the end of the game
			for(auto& move: current_game.moves)
			{
				move.score += (100 - lambda) * (result * result_multiplier) / 100;
				//the side to move changes on each move, so does the perspective of the result
				//a win of one side is the loss of the other side
				result_multiplier *= -1;
			}
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
		int8_t previous_move = Board::passing_index;
		for (const auto& move : game.moves)
		{
			game_file.write((char*)&move.move, sizeof(move.move));
			if (scored)
			{
				game_file.write((char*)&move.score, sizeof(move.score));
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
	PositionPicker pp;
	for (int move = 0; move < length; move++)
	{
		int8_t current_move = buff[move];
		const auto side_to_move = b.get_side_to_move();
		const auto& playfield = b.get_board();

		//save the current input, side to move perspective
		const auto side_to_move_bb = side_to_move == COLOR_WHITE ? playfield.white_bb : playfield.black_bb;
		const auto opposite_side_bb = side_to_move == COLOR_WHITE ? playfield.black_bb : playfield.white_bb;

		bool skip = ((rng::rng() % 11 + 1) + b.get_ply()) <= 10;

		if (!skip)
		{
			pos_entry ret_val;
			//this gets the memory of buff[move+1], which is where the scores start
			//then it casts it to int16_t array, as that's the score type
			//finally, it dereferences the pointer to get the score
			int16_t score = *((int16_t*)(&(buff[move + 1])));
			ret_val.score = score;

			bool found = pp.enter_and_get_position(side_to_move_bb, opposite_side_bb, ret_val);

			if (found)
			{
				//save all symmetries
				for (int symmetry = SYMMETRY_ORIGINAL; symmetry < SYMMETRY_NONE; symmetry++)
				{
					game_file_with_inputs.write((char*)&ret_val.transformed_bitboards[symmetry][0], sizeof(bitboard));
					game_file_with_inputs.write((char*)&ret_val.transformed_bitboards[symmetry][1], sizeof(bitboard));
					//save the score
					if (scored)
					{
						game_file_with_inputs.write((char*)&ret_val.score, sizeof(ret_val.score));
					}
				}
			}
		}
		
		move += 2;
		b.do_move(current_move,false);
		if (b.is_over())
		{
			b.new_game();
		}
	}
	
	const auto& data = pp.get_data();
	for (int i = 0; i < PositionPicker::size; i++)
	{
		const pos_entry& ret_val = data[i];
		if (ret_val.valid)
		{
			//save all symmetries
			for (int symmetry = SYMMETRY_ORIGINAL; symmetry < SYMMETRY_NONE; symmetry++)
			{
				game_file_with_inputs.write((char*)&ret_val.transformed_bitboards[symmetry][0], sizeof(bitboard));
				game_file_with_inputs.write((char*)&ret_val.transformed_bitboards[symmetry][1], sizeof(bitboard));
				//save the score
				if (scored)
				{
					game_file_with_inputs.write((char*)&ret_val.score, sizeof(ret_val.score));
				}
			}
		}
	}
	game_file_with_inputs.close();
}