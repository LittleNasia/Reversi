#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "Search.h"
#include "ClippedReLU.h"
#include "LinearLayer.h"
#include "BoardEvaluator.h"
#include <algorithm>
using namespace std::chrono;

unsigned long long perft = 0;

void pf(Board& pos, int depth)
{
    

    if (depth == 0 || pos.is_over())
    {
        return;
    }

    perft++;
    uint8_t moves[Board::rows * Board::cols / 2];
    std::memcpy(moves, pos.get_moves(), Board::rows * Board::cols / 2);
    int num_moves = pos.get_num_moves();
    int current_move = 0;
    while (moves[current_move] != Board::invalid_index)
    {
        pos.do_move(moves[current_move++]);
        pf(pos, depth - 1);
        pos.undo_move();
    }
    if (!num_moves)
    {
        pos.do_move(Board::invalid_index);
        pf(pos, depth - 1);
        pos.undo_move();
    }
}
 
int main()
{
    NN::BoardEvaluator be;
    search::init();
    Board b;
    std::cout << be.Evaluate(b) << "\n";
   
    for(int d=1; d < 10; d++)
    { 
        int wins = 0;
        int loses = 0;
    for (int game = 0; game < 50000; game++)
    {
        int sum = 0;
        int games = 0;
        int score;
        search::SearchInfo s;
        s.eval_function = evaluate;
        while (!b.is_over())
        {
            //std::cout << "\n\n\nRandom Mover Move:\n";
            b.do_random_move();
            //b.print_board();
            // std::cout << "\nsearch move: ";
            const int move = search::search_move(b, 25, true, score, s);
            //std::cout << "\n\n";
            //std::cout << move << "\n";
            b.do_move(move);
            //print_board();
        }
        //std::cout << "score : " << b.get_score() << "\n";
        if (b.get_score() < 0)
        {
            wins++;
        }
        else
            loses++;
        b.new_game();
        //b.print_board();
        //system("pause");
    } 
    std::cout << "depth: " << d << "    " << wins << "/" << loses << "\n";
    }
    b.print_board();
   // std::cout << "took " << d / 10 << "\n";
}