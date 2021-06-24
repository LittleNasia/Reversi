#include <iostream>
#include "Board.h"
#include <chrono>
#include <half.hpp>
using namespace std::chrono;

int main()
{
    auto start = high_resolution_clock::now();
    auto stop = high_resolution_clock::now();
    unsigned long long x = 12312412412313ULL;
    const unsigned long long bb = 0x7f7f7f7f7f7f7f7full;
    unsigned long long i = 1;
    for (int row = 0; row < 8; row++)
    {
        for (int j = 0; j < 8;j++)
        {
            std::cout << '[' << ((bb & i) ? 'X' : ' ') << ']';
            i <<= 1;
        }
        std::cout << "\n";
    }
    std::cout << "\n\n\n\n";
    x = (x & 0x7f7f7f7f7f7f7f7full) << 1;
    i = 1;
    for (int row = 0; row < 8; row++)
    {
        for (int j = 0; j < 8; j++)
        {
            std::cout << '[' << ((x & i) ? 'X' : ' ') << ']';
            i <<= 1;
        }
        std::cout << "\n";
    }
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << duration.count() << "\n";
}