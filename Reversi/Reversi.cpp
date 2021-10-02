#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "Search.h"
#include "ClippedReLU.h"
#include "LinearLayer.h"
#include "GameGenerator.h"
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
    while (moves[current_move] != Board::passing_index)
    {
        pos.do_move(moves[current_move++]);
        pf(pos, depth - 1);
        pos.undo_move();
    }
    if (!num_moves)
    {
        pos.do_move(Board::passing_index);
        pf(pos, depth - 1);
        pos.undo_move();
    }
}
 
int main()
{
    //NN::BoardEvaluator be;
   // be.test();
    //be.test();
    search::init();
    auto start = high_resolution_clock::now();
    Board b;
    //
    while (!b.is_over())
    {
        std::cout << "ply " << b.get_ply() << " config" << (int)b.get_playfield_config() << "\n";
        std::cout << NN::be.Evaluate(b) << "\n";
        b.do_first_move();
    }
    
    
    GameGenerator gg;
    
    for (int i = 0; i < 1; i++)
    {
        auto games = gg.generate_games(false, true, 1, 4);
        auto filename = gg.save_to_file(games);
        gg.convert_to_input_type(filename);
    }
    
    
    
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << (((float)duration.count() + 1) / 1000000) << "\n\n";
    //b.print_board();
   // std::cout << "took " << d / 10 << "\n";
}