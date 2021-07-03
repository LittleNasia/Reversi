#include <iostream>
#include "Board.h"
#include <chrono>
using namespace std::chrono;

int main()
{
    
    unsigned long long d = 0;
    Board b;
    int sum = 0;
    int games = 0;
    for (int i = 0; i < 10; i++)
    {
        auto start = high_resolution_clock::now();
        
        for (unsigned long long i = 0; i < 10000000; i++)
        {
            int move;
            std::cout << b.getPly() << "\n";
            //std::cout << "board before random move\n";
           // b.printBoard();
           // std::cout << "board after random move\n";
            b.do_random_move();
           // b.printBoard();
           // std::cout << "board after undo move\n";
            b.undoMove();
           // b.printBoard();
            //std::cout << "board after random move\n";
            b.do_random_move();
           // b.printBoard();
          //  std::cout << "\n\n";
            if (b.isOver())
            {
                sum += b.getScore();
                //std::cout << sum << "\n";
                games++;
                b.newGame();
            }
        }
        std::cout << (double)sum / games << "\n";;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        d += duration.count();
        std::cout << duration.count() << "\n";
    }
    b.printBoard();
    std::cout << "took " << d / 10 << "\n";
}