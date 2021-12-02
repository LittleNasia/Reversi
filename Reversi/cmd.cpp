#include "cmd.h"
#include <iostream>
#include <sstream>

#include "Search.h"
namespace cmd
{
	inline thread_local board rootPos;
	void parse(const std::string& cmd)
	{
		if (cmd.substr(0, 2) == "go")
		{
			int depth = search::max_depth;
			int64_t time = search::max_time;
			std::stringstream ss(cmd.substr(2));

			while (true)
			{
				std::string token;
				ss >> token;
				if (token.find("depth") != std::string::npos)
				{
					ss >> depth;
				}
				else if (token.find("time") != std::string::npos)
				{
					ss >> time;
				}
				else
				{
					break;
				}
			}
			
			int score;
			search::search_info s;
			s.eval_function = evaluate;
			s.time = time;
			const auto move = search::search_move(rootPos, depth, true, score, s);
			std::cout << "bestmove " << move << " " << "value " << score << std::endl;

		}
		else if (cmd.substr(0, 8) == "position")
		{
			rootPos = board();
			std::stringstream ss(cmd.substr(8));
			int move = 0;
			while (static_cast<bool>(ss >> move))
			{
				rootPos.do_move_is_legal(move);
			}
		}
		else if (cmd.substr(0, 5) == "clear")
		{
			search::transposition_table.clear();
		}
		else if (cmd.substr(0, 5) == "print")
		{
			rootPos.print_board();
		}
		else if (cmd.substr(0, 5) == "score")
		{
			int score = rootPos.get_score();
			if (score)
			{
				score = ((score > 0) ? 1 : -1);

			}
			std::cout << score << std::endl;

		}
	}

	void loop()
	{
		while (true)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			parse(cmd);
		}
	}
	
}