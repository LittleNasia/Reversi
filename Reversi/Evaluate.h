#pragma once
#include "Board.h"
#include "BoardEvaluator.h"

constexpr unsigned long long center_16_bitmask = 66229406269440ULL;
constexpr unsigned long long center_4_bitmask = 103481868288ULL;
constexpr unsigned long long corners = 0x8100000000000081;
constexpr unsigned long long C_squares = (1ULL << 62) | (1ULL << 55) | (1ULL << 54) | (1ULL << 57) | (1ULL << 49) | (1ULL << 48) | 
(1ULL << 15) | (1ULL << 14) | (1ULL << 6) | (1ULL << 1) | (1ULL << 9) | (1ULL << 8);




const int evaluate(Board& b);
const int evaluate_classical (Board& b);