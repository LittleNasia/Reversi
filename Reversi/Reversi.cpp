#include <iostream>
#include "Board.h"
#include <chrono>
#include <half.hpp>
using namespace std::chrono;

int main()
{
    auto start = high_resolution_clock::now();
    auto stop = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << duration.count() << "\n";
}