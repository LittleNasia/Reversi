#pragma once
#include "nnue_linear_layer.h"
#include "nnue_clipped_relu.h"
#include "nnue_accumulator.h"
#include "board.h"

#include <string>

namespace NN
{ 
	class board_evaluator
	{
#if use_nnue
	public:
		//reads weights from file 
		board_evaluator(std::string& weights_filename);
		//random weights initialization
		board_evaluator();

		int evaluate(const board& b);
		void test();
	private:
		clipped_relu<layer_sizes[1], true> ReLU_layer_1;
		linear_layer<layer_sizes[1], layer_sizes[2]> layer_2;
		clipped_relu<layer_sizes[2], true> ReLU_layer_2;
		linear_layer<layer_sizes[2], layer_sizes[3]> layer_3;
		clipped_relu<layer_sizes[3], true> ReLU_layer_3;
		linear_layer<layer_sizes[3], layer_sizes[4]> layer_output;

#endif
	};

	inline thread_local board_evaluator be;
}
