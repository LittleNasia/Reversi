#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "Search.h"

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
    search::init();
    Board b;
    int wins = 0;
    int loses = 0;
    for (int game = 0; game < 1000; game++)
    {
        unsigned long long d = 0;
        int sum = 0;
        int games = 0;
        while (!b.is_over())
        {
            //std::cout << "\n\n\nRandom Mover Move:\n";
           //b.do_random_move();
            //b.print_board();
            // std::cout << "\nsearch move: ";
            const int move = search::search_move(b, 17, true);
            std::cout << "\n\n";
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
    std::cout << "depth: " << 8 << "    " << wins << "/" << loses << "\n";
    
    b.print_board();
   // std::cout << "took " << d / 10 << "\n";
}