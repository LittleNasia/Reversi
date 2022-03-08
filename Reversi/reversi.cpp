#include "board.h"
#include "Search.h"
#include "nnue_clipped_relu.h"
#include "nnue_linear_layer.h"
#include "game_generator.h"
#include "board_evaluator.h"
#include "position_picker.h"
#include "cmd.h"
#include "conv_layer.h"
#include "cnn_input_layer.h"
#include "cnn.h"
#include "mcts.h"

#include <iostream>
#include <chrono>
#include <cstring>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <iomanip>
using namespace std::chrono;

unsigned long long perft = 0;

void pf(board& pos, int depth)
{
    if (depth == 0 || pos.is_over())
    {
        perft++;
        return;
    }

    
    board::move_type moves[board::rows * board::cols / 2];
    std::memcpy(moves, pos.get_moves(), sizeof(moves));
    int num_moves = pos.get_num_moves();
    int current_move = 0;
    while (moves[current_move] != board::passing_index)
    {
        pos.do_move(moves[current_move++],false);
        pf(pos, depth - 1);
        pos.undo_move();
    }
    if (!num_moves)
    {
        pos.do_move(board::passing_index, false);
        pf(pos, depth - 1);
        pos.undo_move();
    }
}



unsigned long long best_score = 10000000000;
struct win_pct
{
    int score = 0;
    int wins = 0;
};



constexpr int book_size = game_generator::book_size;
int8_t book[book_size][game_generator::max_book_length];
void generate_book()
{
    std::unordered_map<unsigned long long, bool> positions;
    search::search_info s;
    s.eval_function = evaluate;

    for (int game = 0; game < book_size; game++)
    {
        if (!(game % 1000))
        {
            std::cout << game << "\n";
        }
        board pos;
        int8_t curr_game[game_generator::max_book_length];
        std::memset(curr_game, -1, sizeof(curr_game));
        for (int move = 0; move < game_generator::max_book_length; move++)
        {
            int curr_move = pos.do_random_move();
            curr_game[move] = curr_move;
            const auto key = search::hash(pos);
            if (positions.find(key) == std::end(positions))
            {
                int score;
                search::search_move(pos, 1, false, score, s);
                if (std::abs(score) < 100)
                {
                    positions[key] = true;
                    std::memcpy(book[game], curr_game, sizeof(curr_game));
                    break;
                }
            }
            if (move == game_generator::max_book_length-1)
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
#include "evaluate.h"
void check_configs(int max_ply = 100)
{
    int configs[256];
    int bits[6];
    std::memset(configs, 0, sizeof(configs));
    std::memset(bits, 0, sizeof(bits));
    board b;
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
            if (rng::rng()%11 < 8 || (b.get_ply() > 323))
            {
                int score;
                search::search_info s;
                s.eval_function = evaluate;
                int move = search::search_move(b, 4, false, score, s);
                b.do_move(move);
            }
            else
            {
                b.do_random_move(false);
            }
            
            if (b.get_ply() >= max_ply)
            {
                break;
            }
        }
        b.new_game();

    }
    for (int i = 0; i < 256; i++)
    {
        if (!(i % 64))
        {
            std::cout << "\n";
        }
        std::cout << configs[i] << "\n";
    }
    std::cout << "\n";
    for (int bit = 0; bit < 6; bit++)
    {
        std::cout << bits[bit] << "\n";
    }
    std::cout << "\n";
}

void speed_test()
{
    for (int game = 0; game < 25000; game++)
    {
        board pos;
        while (!pos.is_over())
        {
            pos.do_random_move();
#if use_nnue
            xd += NN::be.evaluate(pos);
#endif
        }
        pos.new_game();
    }

}


#include "move_picker.h"
void tune_move_ordering()
{
    static constexpr int max_ply = board::max_ply;
    static constexpr int total_moves = board::passing_index + 1;
    static constexpr int num_configs = 256;
    std::pair<int,int> move_probabilities[max_ply][total_moves];
    std::pair<int, int> configs_moves[num_configs][total_moves];
    std::memset(move_probabilities, 0, sizeof(move_probabilities));
    std::memset(configs_moves, 0, sizeof(configs_moves));
    board b;
    
    auto start = high_resolution_clock::now();
    search::search_info s;
    s.eval_function = evaluate;
    for (int i = 0; i < 30000; i++)
    {
        if (!(i % 100))
        {
            std::cout << i << "\n";
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);
            std::cout << (((float)duration.count() + 1) / 1000000) << "\n\n";
        }
        while (!b.is_over())
        {
            int x;
            int score;
            
            
            auto move = search::search_move(b, 4, false, score, s);
            const auto config = b.get_playfield_config();

             
            move_probabilities[b.get_ply()][move].first++;
            configs_moves[config][move].first++;

            move_picker mp(b);

            while (mp.get_move_count())
            {
                const auto current_move = mp.get_move();
                if (mp.get_move_count() && (current_move == board::passing_index))
                {
                    break;
                }
                
                move_probabilities[b.get_ply()][current_move].second++;
                configs_moves[config][current_move].second++;
            }
            b.do_random_move();
        }
        b.new_game();
    }
    std::cout << "constexpr float ply_move_probability[85][65] = {\n";
    for (int ply = 0; ply < 85; ply++)
    {
        std::cout << "{";
        for (int move = 0; move < 65; move++)
        {
            float result = move_probabilities[ply][move].second != 0 ? (float)move_probabilities[ply][move].first / move_probabilities[ply][move].second : 0;
            std::cout << result <<  ", ";
        }
        std::cout << "},\n";
    }
    std::cout << "}\n\n\nconstexpr float config_move_probability[256][65] = {";
    for (int ply = 0; ply < 256; ply++)
    {
        std::cout << "{";
        for (int move = 0; move < 65; move++)
        {
            float result = configs_moves[ply][move].second != 0 ? (float)configs_moves[ply][move].first / configs_moves[ply][move].second : 0;
            std::cout << result << ", ";
        }
        std::cout << "},\n";
    }
    
}
#include "linear_regression.h"
#include "mlp.h"
//reads an .sinput file and outputs the evaluation 
void test_evaluation(std::string filename)
{
    std::ifstream games_file(filename, std::ios::binary);
    for (int game = 0; game < 100; game++)
    {
        playfield_bitboard curr_bb;
        games_file.read((char*)&curr_bb, sizeof(bitboard) * 2);
        board b;
        b.set_board_state(curr_bb);
        //b.print_board();
        int16_t score;
        games_file.read((char*)&score, sizeof(int16_t));
        std::cout <<"test " <<  evaluate(b) << " " << score << "\n";
    }
}

int main()
{
    search::init();
    board b;
    CNN::cnn network;
    linear_regression::load_weights();

    auto start_perft = high_resolution_clock::now();
    pf(b, 12);
    auto stop_perft = high_resolution_clock::now();
    auto duration_perft = duration_cast<microseconds>(stop_perft - start_perft);
    std::cout << "perft " << (((float)duration_perft.count() + 1) / 1000000) << "\n\n";
    cmd::loop();
}   
    
    //int configs[256];
    //int bits[6];
    //std::memset(configs, 0, sizeof(configs));
    //std::memset(bits, 0, sizeof(bits));
    //board b;
    //search::search_info s;
    //s.eval_function = evaluate;
    //
    
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
