#pragma once
#include <fstream>
#include <string>
#include <vector>

#include "Board.h"
#include "Search.h"
#include "Utils.h"

inline constexpr int8_t game_begin = -1;
//the file format is in the following:

//games with no rescoring, each move is one byte, they are all one after another
//the games start with game_begin move as the first move, it's just an equivalent of "no move" for starting position
//it's made so that number of moves matches with number of scores (startpos has no moves, but has a score, so it's assigned a "game_begin" move)
//the format is <filename>.nsbin

//games where each position is given a score, the format is:
//all 8 bit moves in a game, followed by game_moves 16 bit evaluations, the game_begin move for startpos still applies
//the format is <filename>.sbin

//scores the evaluation of the position and the move made in this position
struct scored_move
{
	int8_t move;
	int16_t score;
};

struct NN_training_entry
{
	static constexpr int input_size = 64 * 4;
	float input[input_size];
	float score;
};

struct Game
{
	scored_move moves[80];
	int game_moves = 0;
	bool scored = false;
};


class GameGenerator
{
public:
	enum file_type
	{
		//nsbin, purely moves
		FILE_UNSCORED,
		//moves followed by scores
		FILE_SCORED,
		//128 bit of bitboards, followed by scores
		FILE_INPUTS
	};
	//games to be written to a single file
	static constexpr int games_per_file = 10000;
	//game will terminate early even if it doesn't finish before this number of moves
	static constexpr int max_game_size = 70;
	//chance for a random move, max value is 1028 so the unit is something like a permil but slightly less
	static constexpr int random_move_chance = 157;
	//how many random moves can be played in a selfplay game
	static constexpr int max_random_moves = 4; 
	//if game reaches this ply, random moves will not be used anymore
	static constexpr int max_random_move_ply = 20;
	//how much of a saved score is evaluation, and how much is the pure game result
	//lambda = 0 -> use purely game results, lambda = 100 -> use purely evaluation, anything inbetween is the interpolation of the two
	static constexpr int lambda = 100;

	GameGenerator();


	//takes the filename as argument, returns the vector of games, can be for example rescored or just read for fun
	std::vector<Game> parse_game_file(const std::string& filename, bool& found_scores);

	//generates games_per_file games, can generate totally random games (then the following arguments are ignored)
	//search_depth is the depth of the search used to generate games
	std::vector<Game> generate_games(const bool use_random_movers, const bool save_scores, const int search_depth, int num_threads);

	//saves games to file
	std::string save_to_file(const std::vector<Game>& games);

	void convert_to_input_type(const std::string& filename);

	//resets the internal state, will overwrite the previous game files 
	void reset() {
		current_file_index = 0;
	}
private:
	std::vector<Game> game_generator_worker(const bool use_random_movers, const bool save_scores, const int search_depth);

	int current_file_index = 0;
	const std::string name = "games";
};

