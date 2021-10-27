#include <iostream>
#include "Board.h"
#include <chrono>
#include <cstring>
#include "Search.h"
#include "ClippedReLU.h"
#include "LinearLayer.h"
#include "GameGenerator.h"
#include "BoardEvaluator.h"
#include "PositionPicker.h"

#include <unordered_set>
#include <algorithm>
#include <random>
using namespace std::chrono;

unsigned long long perft = 0;

void pf(Board& pos, int depth)
{
    if (depth == 0 || pos.is_over())
    {
        perft++;
        return;
    }

    
    uint8_t moves[Board::rows * Board::cols / 2];
    std::memcpy(moves, pos.get_moves(), Board::rows * Board::cols / 2);
    int num_moves = pos.get_num_moves();
    int current_move = 0;
    while (moves[current_move] != Board::passing_index)
    {
        pos.do_move(moves[current_move++],false);
        pf(pos, depth - 1);
        pos.undo_move();
    }
    if (!num_moves)
    {
        pos.do_move(Board::passing_index, false);
        pf(pos, depth - 1);
        pos.undo_move();
    }
}

enum game_phase
{
    PHASE_EARLY,
    PHASE_MID,
    PHASE_LATE,
    PHASE_END
};

struct masks
{
    bitboard m[6];
    int indices[6][2];
    unsigned long long score;
};

struct population
{
    masks ms[2000];
};

void mutate(masks& m)
{
    int swaps_to_make = rng::rng() % 10 + 1;
    for (int i = 0; i < swaps_to_make; i++)
    {
        int mask_first = rng::rng() % 6;
        int mask_second = rng::rng() % 6;
        std::swap(m.indices[mask_first][rng::rng() % 2], m.indices[mask_second][rng::rng() % 2]);
    }
    for (int mask = 0; mask < 6; mask++)
    {
        m.m[mask] = 0ULL;
        for (int index = 0; index < 2; index++)
        {
            if (m.indices[mask][index] == -1)
            {
                continue;
            }
            m.m[mask] |= (1ULL << m.indices[mask][index]);
        }
    }
}



const uint8_t get_playfield_config(const playfield_bitboard& bb, masks& m)
{
    bitboard combined_bb = bb.white_bb | bb.black_bb;
    const int popcnt = __popcnt64(combined_bb);
    int game_phase_index = 0;
    static constexpr bitboard masks[4][6] =
    {
        {
        (1ULL << 30 | 1ULL << 17 | 1ULL << 54 | 1ULL << 50),
        (1ULL << 53 | 1ULL << 33 | 1ULL << 46 | 1ULL << 25),
        (1ULL << 38 | 1ULL << 52 | 1ULL << 10 | 1ULL << 51),
        (1ULL << 9 | 1ULL << 13 | 1ULL << 45),
        (1ULL << 22 | 1ULL << 12 | 1ULL << 11 | 1ULL << 41),
        (1ULL << 14 | 1ULL << 49 | 1ULL << 18)
        },
        {
        (1ULL << 0 | 1ULL << 63 | 1ULL << 51 | 1ULL << 5),
        (1ULL << 14 | 1ULL << 7 | 1ULL << 4 | 1ULL << 23),
        (1ULL << 32 | 1ULL << 39 | 1ULL << 31 | 1ULL << 24),
        (1ULL << 47 | 1ULL << 54 | 1ULL << 53 | 1ULL << 52),
        (1ULL << 3 | 1ULL << 9 | 1ULL << 16 | 1ULL << 2),
        (1ULL << 50 | 1ULL << 56 | 1ULL << 40 | 1ULL << 49)
        },
        {
        (1ULL << 57 | 1ULL << 56),
        (1ULL << 15 | 1ULL << 7),
        (1ULL << 8 | 1ULL << 1),
        (1ULL << 62 | 1ULL << 55),
        (1ULL << 63 | 1ULL << 48),
        (1ULL << 6 | 1ULL << 0)
        },
        {
        (1ULL << 55 | 1ULL << 1),
        (1ULL << 48 | 1ULL << 63),
        (1ULL << 7 | 1ULL << 15),
        (1ULL << 0 | 1ULL << 8),
        (1ULL << 57 | 1ULL << 56),
        (1ULL << 62 | 1ULL << 6)
        }
    };
    game_phase phase = PHASE_EARLY;
    if (popcnt >= 49)
    {
        game_phase_index = 64 * 3;
        phase = PHASE_END;
        //return 0;
    }
    else if (popcnt >= 34)
    {
        game_phase_index = 64 * 2;
        phase = PHASE_LATE;
        uint8_t result = 0;
        for (int bit_index = 0; bit_index < 6; bit_index++)
        {
            if ((combined_bb & masks[phase][bit_index]))
            {
                result |= (1 << bit_index);
            }
        }
        return result + game_phase_index;
    }
    else if (popcnt >= 19)
    {
        game_phase_index = 64 * 1;
        phase = PHASE_MID;
        uint8_t result = 0;
        for (int bit_index = 0; bit_index < 6; bit_index++)
        {
            if ((combined_bb & masks[phase][bit_index]))
            {
                result |= (1 << bit_index);
            }
        }
        return result + game_phase_index;
    }
    else
    {
        game_phase phase = PHASE_EARLY;
        uint8_t result = 0;
        for (int bit_index = 0; bit_index < 6; bit_index++)
        {
            if ((combined_bb & masks[phase][bit_index]))
            {
                result |= (1 << bit_index);
            }
        }
        return result;
    }
    
    uint8_t result = 0;
    combined_bb = ~combined_bb;
    for (int bit_index = 0; bit_index < 6; bit_index++)
    {
        //print_bitboard(masks[bit_index]);
        //std::cout << m.m[bit_index] << "\n";
        if ((combined_bb & m.m[bit_index]))
        {
            result |= (1 << bit_index);
            //std::cout << "yes\n";
        }
        else
        {
            //std::cout << "no\n";
        }
    }
    return game_phase_index + result;
}


unsigned long long best_score = 10000000000;
struct win_pct
{
    int score = 0;
    int wins = 0;
};



constexpr int book_size = GameGenerator::book_size;
int8_t book[book_size][GameGenerator::max_book_length];
void generate_book()
{
    std::unordered_map<unsigned long long, bool> positions;
    search::SearchInfo s;
    s.eval_function = evaluate;

    for (int game = 0; game < book_size; game++)
    {
        if (!(game % 1000))
        {
            std::cout << game << "\n";
        }
        Board pos;
        int8_t curr_game[GameGenerator::max_book_length];
        std::memset(curr_game, -1, sizeof(curr_game));
        for (int move = 0; move < GameGenerator::max_book_length; move++)
        {
            int curr_move = pos.do_random_move();
            curr_game[move] = curr_move;
            const auto key = search::hash(pos);
            if (positions.find(key) == std::end(positions))
            {
                int score;
                search::search_move(pos, 3, false, score, s);
                if (std::abs(score) < 100)
                {
                    positions[key] = true;
                    std::memcpy(book[game], curr_game, sizeof(curr_game));
                    break;
                }
            }
            if (move == GameGenerator::max_book_length-1)
            {
                game--;
            }
        }
        pos.new_game();
    }

    std::ofstream book_file("book.bin", std::ios::binary);
    book_file.write((char*)book, sizeof(book));
    book_file.close();
}

void check_configs()
{
    int configs[256];
    int bits[6];
    std::memset(configs, 0, sizeof(configs));
    std::memset(bits, 0, sizeof(bits));
    Board b;
    for (int i = 0; i < 5000; i++)
    {
        while (!b.is_over())
        {
            int config = b.get_playfield_config();
            //std::cout << config << "\n";
            configs[config] += 1;
            int cfg = config;
            for (int i = 0; i < 6; i++)
            {
                if ((1 << i) & cfg)
                {
                    bits[i]++;
                }
            }
            if (b.get_side_to_move() == COLOR_BLACK)
            {
                b.do_random_move(false);
            }
            else
            {
                b.do_random_move(false);
            }
        }
        b.new_game();

    }
    for (int i = 0; i < 256; i++)
    {
        std::cout << configs[i] << "\n";
    }
    for (int bit = 0; bit < 6; bit++)
    {
        std::cout << bits[bit] << "\n";
    }
}

int main()
{
    search::init();
    NN::be.test();
    //std::cout << (-127 >> 1);


    int wins = 0;
    int draws = 0;
    int loses = 0;

    //check_configs();
    
    unsigned long long best_score = 100000000000000;
    int best_masks[6][4];
    std::random_device rd;
    std::mt19937 g(rd());
    //constants


	auto start = high_resolution_clock::now();
	Board b;
	search::SearchInfo s;
	s.eval_function = evaluate;
	for (int i = 0; i < 1000; i++)
    {
       if (!(i % 100))
       {
           std::cout << i << "\n";
           auto stop = high_resolution_clock::now();
           auto duration = duration_cast<microseconds>(stop - start);
           std::cout << (((float)duration.count() + 1) / 1000000) << "\n\n";
       }
       std::vector<scored_move> m;
       while (!b.is_over())
       {
           //if (b.get_side_to_move() == COLOR_WHITE)
           //{
           //    if(b.get_ply()<8)
           //     b.do_random_move();
           //    else
           //    {
           //        int score;
           //        search::transposition_table.clear();
           //        int move = search::search_move(b, 3, false, score, s);

           //        //std::cout << "\n\n";
           //        b.do_move(move);
           //    }
           //}
           //else
           //{
           //    int score;
           //   // std::cout <<"\n" <<  NN::be.Evaluate(b) << "\n";
           //    //if ((b.get_ply() == 0 && (i==1)))
           //    {
           //        //search::search_move(b, 15, true, score, s);
           //    }
           //    int move = search::search_move(b, 7, false, score, s);
           //    
           //    //std::cout << "\n\n";
           //    b.do_move(move);
           //}
           /*if (false && (b.get_ply() < 10 && rng::rng()%2) || (b.get_ply() > 10 && !(rng::rng() % 10)))
           {
               b.do_random_move();
           }
           else
           {
               int score = 0;
               int move = search::search_move(b, 1, false, score, s);
               vals[b.get_ply()][move].first++;
               configs_moves[b.get_playfield_config()][move].first++;
               const auto moves = b.get_moves();
               for (int move = 0; move < b.get_num_moves(); move++)
               {
                   vals[b.get_ply()][moves[move]].second++;
                   configs_moves[b.get_playfield_config()][moves[move]].second++;
               }
               b.do_move(move);
           }*/
           int x;
           int score;
           scored_move curr_scored_move;
           if (b.get_side_to_move() == COLOR_WHITE)
           {
               if (false)// && (b.get_ply() < 0) || (rng::rng()%10))
               {
                   search::transposition_table.clear();
                   int move = search::search_move(b, 4, false, score, s);
                   b.do_move(move);
               }
               if (true)
               {
                   b.do_random_move();
               }
           }
           else
           {
               int move = search::search_move(b, 1, false, score, s);
               b.do_move(move);
           }
       }
       int result = 0;
       if (b.get_score() < 0)
       {
           wins += 1;
           result = -1;
       }
       else if (b.get_score() > 0)
       {
           loses += 1;
           result = 1;
       }
       else
       {
           draws += 1;
           result = 0;
       }

       b.new_game();
    }

    //generate_book();




    GameGenerator gg;
	std::cout << wins << "/" << draws << "/" << loses << "\n\n";
    for (int i = 0; i < 5; i++)
    {
        auto games = gg.generate_games(false, true, 3, 4);
        auto filename = gg.save_to_file(games);
        gg.convert_to_input_type(filename);
    }

}   
    
    //int configs[256];
    //int bits[6];
    //std::memset(configs, 0, sizeof(configs));
    //std::memset(bits, 0, sizeof(bits));
    //Board b;
    //search::SearchInfo s;
    //s.eval_function = evaluate;
    //
    //std::pair<int,int> vals[100][65];
    //std::pair<int, int> configs_moves[256][65];
    //std::memset(vals, 0, sizeof(vals));
    //std::memset(configs_moves, 0, sizeof(configs_moves));
    //
    //std::unordered_map<int, win_pct> scores;
    //auto start = high_resolution_clock::now();
    //for (int i = 0; i < 1000; i++)
    //{
    //    if (!(i % 100))
    //    {
    //        std::cout << i << "\n";
    //        auto stop = high_resolution_clock::now();
    //        auto duration = duration_cast<microseconds>(stop - start);
    //        std::cout << (((float)duration.count() + 1) / 1000000) << "\n\n";
    //    }
    //    std::vector<scored_move> m;
    //    while (!b.is_over())
    //    {
    //        //if (b.get_side_to_move() == COLOR_WHITE)
    //        //{
    //        //    if(b.get_ply()<8)
    //        //     b.do_random_move();
    //        //    else
    //        //    {
    //        //        int score;
    //        //        search::transposition_table.clear();
    //        //        int move = search::search_move(b, 3, false, score, s);

    //        //        //std::cout << "\n\n";
    //        //        b.do_move(move);
    //        //    }
    //        //}
    //        //else
    //        //{
    //        //    int score;
    //        //   // std::cout <<"\n" <<  NN::be.Evaluate(b) << "\n";
    //        //    //if ((b.get_ply() == 0 && (i==1)))
    //        //    {
    //        //        //search::search_move(b, 15, true, score, s);
    //        //    }
    //        //    int move = search::search_move(b, 7, false, score, s);
    //        //    
    //        //    //std::cout << "\n\n";
    //        //    b.do_move(move);
    //        //}
    //        /*if (false && (b.get_ply() < 10 && rng::rng()%2) || (b.get_ply() > 10 && !(rng::rng() % 10)))
    //        {
    //            b.do_random_move();
    //        }
    //        else
    //        {
    //            int score = 0;
    //            int move = search::search_move(b, 1, false, score, s);
    //            vals[b.get_ply()][move].first++;
    //            configs_moves[b.get_playfield_config()][move].first++;
    //            const auto moves = b.get_moves();
    //            for (int move = 0; move < b.get_num_moves(); move++)
    //            {
    //                vals[b.get_ply()][moves[move]].second++;
    //                configs_moves[b.get_playfield_config()][moves[move]].second++;
    //            }
    //            b.do_move(move);
    //        }*/
    //        int x;
    //        int score;
    //        scored_move curr_scored_move;
    //        if (b.get_side_to_move() == COLOR_WHITE)
    //        {
    //            if (false)// && (b.get_ply() < 0) || (rng::rng()%10))
    //            {
    //                search::transposition_table.clear();
    //                int move = search::search_move(b, 4, false, score, s);
    //                b.do_move(move);
    //            }
    //            if (true)
    //            {
    //                b.do_random_move();
    //            }
    //        }
    //        else
    //        {
    //            int move = search::search_move(b, 1, false, score, s);
    //            b.do_move(move);
    //        }
    //    }
    //    int result = 0;
    //    if (b.get_score() < 0)
    //    {
    //        wins += 1;
    //        result = -1;
    //    }
    //    else if (b.get_score() > 0)
    //    {
    //        loses += 1;
    //        result = 1;
    //    }
    //    else
    //    {
    //        draws += 1;
    //        result = 0;
    //    }
    //    for (const auto& val : m)
    //    {
    //        if (scores.find(val.score) != std::end(scores))
    //        {
    //            scores[val.score].score += result;
    //            scores[val.score].wins += 1;
    //        }
    //        else
    //        {
    //            scores[val.score] = { result,1 };
    //        }
    //    }
    //    b.new_game();
    //}
    ///*std::cout << "constexpr float ply_move_probability[85][65] = {\n";
    //for (int ply = 0; ply < 85; ply++)
    //{
    //    std::cout << "{";
    //    for (int move = 0; move < 65; move++)
    //    {
    //        float result = vals[ply][move].second != 0 ? (float)vals[ply][move].first / vals[ply][move].second : 0;
    //        std::cout << result <<  ", ";
    //    }
    //    std::cout << "},\n";
    //}
    //std::cout << "}\n\n\nconstexpr float config_move_probability[256][65] = {";
    //for (int ply = 0; ply < 256; ply++)
    //{
    //    std::cout << "{";
    //    for (int move = 0; move < 65; move++)
    //    {
    //        float result = configs_moves[ply][move].second != 0 ? (float)configs_moves[ply][move].first / configs_moves[ply][move].second : 0;
    //        std::cout << result << ", ";
    //    }
    //    std::cout << "},\n";
    //}
    //std::cout << "}\n";*/
    //std::cout << wins << "/" << draws << "/" << loses << "\n\n";
    //auto stop = high_resolution_clock::now();
    //auto duration = duration_cast<microseconds>(stop - start);
    //std::cout << (((float)duration.count() + 1) / 1000000) << "\n\n";
    ////for (int i = 0; i < 256; i++)
    //{
    //    //std::cout << configs[i] << "\n";
    //}
    
    //
    //
    //
    
    //b.print_board();
   // std::cout << "took " << d / 10 << "\n";
