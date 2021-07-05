#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "MostPiecesTaken.h"
using namespace std::chrono;

unsigned long long perft = 0;

void search(Board& pos, int depth)
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
        search(pos, depth - 1);
        pos.undoMove();
    }
    if (!num_moves)
    {
        pos.do_move(Board::invalid_index);
        search(pos, depth - 1);
        pos.undoMove();
    }
}
 
int main()
{
    unsigned long long d = 0;
    Board b;
    int sum = 0;
    int games = 0;
    auto start = high_resolution_clock::now();
    search(b, 12);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    d += duration.count();
    std::cout << perft << "\n";
    std::cout << perft/duration.count() << "\n";
    
    b.printBoard();
   // std::cout << "took " << d / 10 << "\n";
}