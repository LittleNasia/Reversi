#include <iostream>
#include "Board.h"
#include <chrono>
#include <half.hpp>
using namespace std::chrono;

int main()
{
    auto start = high_resolution_clock::now();
    auto stop = high_resolution_clock::now();
    Board b;
    while (true)
    {
        int move;
        b.printBoard();
        std::cin >> move;
        b.do_random_move();
    }
    
   
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << duration.count() << "\n";
}