#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "MostPiecesTaken.h"
#include "Search.h"

using namespace std::chrono;

unsigned long long perft = 0;

void pf(Board& pos, int depth)
{
    if (depth == 0 || pos.isOver())
    {
        
        return;
    }
    perft++;
    uint8_t moves[Board::rows * Board::cols / 2];
    std::memcpy(moves, pos.getMoves(), Board::rows * Board::cols / 2);
    int num_moves = pos.getNumMoves();
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
    int wins = 0;
    int loses = 0;
    Board b;
    for (int depth = 10; depth < 11; depth++)
    {
        int wins = 0;
        int loses = 0;
        for (int game = 0; game < 1000; game++)
        {
            unsigned long long d = 0;
            int sum = 0;
            int games = 0;
            while (!b.isOver())
            {
                //std::cout << "\n\n\nRandom Mover Move:\n";
                //b.do_random_move();
                b.print_board();
               // std::cout << "\nsearch move: ";
                const int move = search::search_move(b, 17);
                //std::cout << "\n\n";
                //std::cout << move << "\n";
                b.do_move_is_legal(move);
                b.print_board();
            }
            //std::cout << "score : " << b.getScore() << "\n";
            if (b.getScore() < 0)
            {
                wins++;
            }
            else
                loses++;
            b.new_game();
            //b.print_board();
            //system("pause");
        } 
        std::cout << "depth: " << depth << "    " << wins << "/" << loses << "\n";
    }
    b.print_board();
   // std::cout << "took " << d / 10 << "\n";
}